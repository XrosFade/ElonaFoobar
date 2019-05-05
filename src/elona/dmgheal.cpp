#include "dmgheal.hpp"
#include "../snail/android.hpp"
#include "ability.hpp"
#include "activity.hpp"
#include "animation.hpp"
#include "area.hpp"
#include "audio.hpp"
#include "buff.hpp"
#include "character.hpp"
#include "character_status.hpp"
#include "config/config.hpp"
#include "debug.hpp"
#include "dmgheal.hpp"
#include "draw.hpp"
#include "element.hpp"
#include "elona.hpp"
#include "event.hpp"
#include "fov.hpp"
#include "i18n.hpp"
#include "item.hpp"
#include "lua_env/event_manager.hpp"
#include "lua_env/interface.hpp"
#include "lua_env/lua_env.hpp"
#include "lua_env/lua_event/lua_event_calc_character_damage.hpp"
#include "lua_env/lua_event/lua_event_character_damaged.hpp"
#include "lua_env/lua_event/lua_event_character_killed.hpp"
#include "map.hpp"
#include "map_cell.hpp"
#include "mef.hpp"
#include "message.hpp"
#include "optional.hpp"
#include "quest.hpp"
#include "random.hpp"
#include "status_ailment.hpp"
#include "variables.hpp"



namespace
{


int dmg_at_m141 = 0;



void end_dmghp(const Character& victim)
{
    if (victim.is_hung_on_sand_bag())
    {
        if (is_in_fov(victim))
        {
            txt(u8"("s + dmg_at_m141 + u8")"s + i18n::space_if_needed());
            if (rnd(20) == 0)
            {
                int picked = rnd(6);
                txt(i18n::s.get_enum(
                    "core.locale.damage.sand_bag", picked, victim));
            }
        }
    }
    game_data.proc_damage_events_flag = 0;
    txt3rd = 0;
}



void dmgheal_death_by_backpack(Character& chara)
{
    int heaviest_item_index = -1;
    int heaviest_weight = 0;
    std::string heaviest_item_name;

    for (const auto& cnt : items(chara.index))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].weight > heaviest_weight)
        {
            heaviest_item_index = cnt;
            heaviest_weight = inv[cnt].weight;
        }
    }
    if (heaviest_item_index == -1)
    {
        heaviest_item_name = i18n::s.get_enum_property(
            "core.locale.death_by.other", "backpack", 6);
    }
    else
    {
        heaviest_item_name = itemname(heaviest_item_index);
    }
    txt(i18n::s.get_enum_property(
        "core.locale.death_by.other", "text", 6, chara, heaviest_item_name));
    if (chara.index == 0)
    {
        ndeathcause = i18n::s.get_enum_property(
            "core.locale.death_by.other", "death_cause", 6, heaviest_item_name);
    }
}



/**
 * Calculates the new state to set the character to at the end of damage_hp().
 */
Character::State dmgheal_set_death_status(Character& victim)
{
    Character::State new_state = victim.state();

    if (victim.character_role == 0)
    {
        new_state = Character::State::empty;
    }
    else if (victim.character_role == 13)
    {
        new_state = Character::State::adventurer_dead;
        victim.time_to_revive = game_data.date.hours() + 24 + rnd(12);
    }
    else
    {
        new_state = Character::State::villager_dead;
        victim.time_to_revive = game_data.date.hours() + 48;
    }

    if (victim.index != 0)
    {
        if (victim.index < 16)
        {
            new_state = Character::State::pet_dead;
            chara_modify_impression(victim, -10);
            victim.current_map = 0;
            if (victim.is_escorted() == 1)
            {
                event_add(15, victim.id);
                new_state = Character::State::empty;
            }
            if (victim.is_escorted_in_sub_quest() == 1)
            {
                new_state = Character::State::empty;
            }
        }
    }

    return new_state;
}



} // namespace



namespace elona
{



int damage_hp(
    Character& victim,
    int amount,
    int damage_source,
    int element,
    int element_power)
{
    int damage_level = 0;
    elona_vector1<int> p_at_m141;
    int gained_exp = 0;
    bool attacker_is_player = false;

    optional<Character&> attacker = none;
    if (damage_source >= 0)
    {
        attacker = cdata[damage_source];
    }

    if (txt3rd == 0)
    {
        attacker_is_player = damage_source == 0;
    }
    else
    {
        attacker_is_player = false;
    }
    if (victim.state() != Character::State::alive)
    {
        end_dmghp(victim);
        return 0;
    }
    dmg_at_m141 = amount * (1 + (victim.furious > 0));
    if (attacker)
    {
        if (attacker->furious > 0)
        {
            dmg_at_m141 *= 2;
        }
    }
    if (element != 0 && element < 61)
    {
        int resistance = sdata(element, victim.index) / 50;
        if (resistance < 3)
        {
            dmg_at_m141 =
                dmg_at_m141 * 150 / clamp((resistance * 50 + 50), 40, 150);
        }
        else if (resistance < 10)
        {
            dmg_at_m141 = dmg_at_m141 * 100 / (resistance * 50 + 50);
        }
        else
        {
            dmg_at_m141 = 0;
        }
        dmg_at_m141 = dmg_at_m141 * 100 / (sdata(60, victim.index) / 2 + 50);
    }
    if (attacker_is_player)
    {
        if (critical)
        {
            snd_at("core.atk2", victim.position);
        }
        else
        {
            snd_at("core.atk1", victim.position);
        }
    }
    if (victim.wet > 0)
    {
        if (element == 50 || damage_source == -9)
        {
            dmg_at_m141 = dmg_at_m141 / 3;
        }
        if (element == 52)
        {
            dmg_at_m141 = dmg_at_m141 * 3 / 2;
        }
    }
    if (element)
    {
        if (element != 60)
        {
            if (victim.is_immune_to_elemental_damage())
            {
                dmg_at_m141 = 0;
            }
        }
    }
    if (victim.is_metal())
    {
        dmg_at_m141 = rnd(dmg_at_m141 / 10 + 2);
    }
    if (victim.is_contracting_with_reaper())
    {
        if (victim.hp - dmg_at_m141 <= 0)
        {
            if (clamp(
                    25 + buff_find(victim, "core.contingency")->power / 17,
                    25,
                    80) >= rnd(100))
            {
                dmg_at_m141 *= -1;
            }
        }
    }
    if (victim.nullify_damage > 0)
    {
        if (victim.nullify_damage > rnd(100))
        {
            dmg_at_m141 = 0;
        }
    }
    if (element == 658)
    {
        dmg_at_m141 = amount;
    }
    rtdmg = dmg_at_m141;

    if (victim.index == 0 && cdata.player().god_id == core_god::opatos)
    {
        dmg_at_m141 = dmg_at_m141 * 90 / 100;
    }

    if (debug::voldemort && victim.index == 0)
    {
        dmg_at_m141 = 0;
    }

    auto result =
        lua::lua->get_event_manager().trigger(lua::CalcCharacterDamageEvent(
            victim, dmg_at_m141, element, damage_source));
    if (auto damage = result.optional<int>("damage"))
    {
        dmg_at_m141 = *damage;
    }
    else if (result.blocked())
    {
        return 1;
    }

    victim.hp -= dmg_at_m141;

    lua::lua->get_event_manager().trigger(lua::CharacterDamagedEvent(
        victim, dmg_at_m141, element, damage_source));

    if (is_in_fov(victim))
    {
        const auto color_id = element_color_id(ele);
        const auto r = static_cast<uint8_t>(255 - c_col(0, color_id));
        const auto g = static_cast<uint8_t>(255 - c_col(1, color_id));
        const auto b = static_cast<uint8_t>(255 - c_col(2, color_id));
        add_damage_popup(std::to_string(dmg_at_m141), victim.index, {r, g, b});
    }


    if (element == 56)
    {
        if (attacker)
        {
            if (dmg_at_m141 > 0)
            {
                heal_hp(
                    *attacker,
                    clamp(
                        rnd(dmg_at_m141 * (150 + element_power * 2) / 1000 +
                            10),
                        1,
                        attacker->max_hp / 10 + rnd(5)));
            }
        }
    }
    if (victim.index == 0)
    {
        game_data.player_cellaccess_check_flag = 0;
        if (victim.hp < 0)
        {
            if (event_id() != -1)
            {
                if (event_id() != 21)
                {
                    victim.hp = 1;
                }
            }
            if (game_data.current_map == mdata_t::MapId::pet_arena)
            {
                victim.hp = 1;
            }
        }
    }
    if (dmg_at_m141 <= 0)
    {
        damage_level = -1;
    }
    else
    {
        damage_level = dmg_at_m141 * 6 / victim.max_hp;
    }
    if (victim.hp < 0)
    {
        if (victim.index < 16)
        {
            for (int cnt = 0; cnt < 16; ++cnt)
            {
                if (victim.index == cnt)
                {
                    continue;
                }
                if (cdata[cnt].state() != Character::State::alive)
                {
                    continue;
                }
                if (cdata[cnt].has_lay_hand() == 0)
                {
                    continue;
                }
                if (cdata[cnt].is_lay_hand_available() == 0)
                {
                    continue;
                }
                cdata[cnt].is_lay_hand_available() = false;
                txt(i18n::s.get("core.locale.damage.lay_hand", cdata[cnt]),
                    Message::color{ColorIndex::cyan});
                txt(i18n::s.get("core.locale.damage.is_healed", victim));
                victim.hp = victim.max_hp / 2;
                animode = 100 + victim.index;
                MiracleAnimation().play();
                snd("core.pray2");
                break;
            }
        }
        else if (victim.is_hung_on_sand_bag())
        {
            victim.hp = victim.max_hp;
        }
    }
    if (victim.hp >= 0)
    {
        if (damage_level > 1)
        {
            spillblood(victim.position.x, victim.position.y, 1 + rnd(2));
        }
        if (game_data.proc_damage_events_flag == 1)
        {
            txteledmg(0, attacker_is_player ? 0 : -1, victim.index, element);
            goto label_1369_internal;
        }
        if (damage_level > 0)
        {
            if (victim.max_hp / 2 > victim.hp)
            {
                ++damage_level;
                if (victim.max_hp / 4 > victim.hp)
                {
                    ++damage_level;
                    if (victim.max_hp / 10 > victim.hp)
                    {
                        ++damage_level;
                    }
                }
            }
        }
        if (game_data.proc_damage_events_flag == 2)
        {
            Message::instance().continue_sentence();
            if (damage_level == -1)
            {
                txt(i18n::s.get(
                    "core.locale.damage.levels.scratch",
                    victim,
                    attacker_is_player));
            }
            if (damage_level == 0)
            {
                txt(i18n::s.get(
                        "core.locale.damage.levels.slightly",
                        victim,
                        attacker_is_player),
                    Message::color{ColorIndex::orange});
            }
            if (damage_level == 1)
            {
                txt(i18n::s.get(
                        "core.locale.damage.levels.moderately",
                        victim,
                        attacker_is_player),
                    Message::color{ColorIndex::gold});
            }
            if (damage_level == 2)
            {
                txt(i18n::s.get(
                        "core.locale.damage.levels.severely",
                        victim,
                        attacker_is_player),
                    Message::color{ColorIndex::light_red});
            }
            if (damage_level >= 3)
            {
                txt(i18n::s.get(
                        "core.locale.damage.levels.critically",
                        victim,
                        attacker_is_player),
                    Message::color{ColorIndex::red});
            }
            rowact_check(victim.index);
            goto label_1369_internal;
        }
        if (damage_level == 1)
        {
            if (is_in_fov(victim))
            {
                txt(i18n::s.get("core.locale.damage.reactions.screams", victim),
                    Message::color{ColorIndex::gold});
            }
        }
        if (damage_level == 2)
        {
            if (is_in_fov(victim))
            {
                txt(i18n::s.get(
                        "core.locale.damage.reactions.writhes_in_pain", victim),
                    Message::color{ColorIndex::light_red});
            }
        }
        if (damage_level >= 3)
        {
            if (is_in_fov(victim))
            {
                txt(i18n::s.get(
                        "core.locale.damage.reactions.is_severely_hurt",
                        victim),
                    Message::color{ColorIndex::red});
            }
        }
        if (dmg_at_m141 < 0)
        {
            if (victim.hp > victim.max_hp)
            {
                victim.hp = victim.max_hp;
            }
            if (is_in_fov(victim))
            {
                txt(i18n::s.get("core.locale.damage.is_healed", victim),
                    Message::color{ColorIndex::blue});
            }
        }
    label_1369_internal:
        rowact_check(victim.index);
        if (victim.hp < victim.max_hp / 5)
        {
            if (victim.index != 0)
            {
                if (victim.fear == 0)
                {
                    bool runs_away = false;
                    if (victim.is_immune_to_fear() == 0)
                    {
                        if (dmg_at_m141 * 100 / victim.max_hp + 10 > rnd(200))
                        {
                            runs_away = true;
                        }
                        else
                        {
                            runs_away = false;
                        }
                        if (attacker_is_player)
                        {
                            if (trait(44)) // Gentle Face
                            {
                                runs_away = false;
                            }
                        }
                        if (runs_away)
                        {
                            victim.fear = rnd(20) + 5;
                            if (is_in_fov(victim))
                            {
                                txt(i18n::s.get(
                                        "core.locale.damage.runs_away_in_"
                                        "terror",
                                        victim),
                                    Message::color{ColorIndex::blue});
                            }
                        }
                    }
                }
            }
        }
        if (element)
        {
            if (element == 59)
            {
                if (rnd(10) < element_power / 75 + 4)
                {
                    dmgcon(
                        victim.index,
                        StatusAilment::blinded,
                        rnd(element_power / 3 * 2 + 1));
                }
                if (rnd(20) < element_power / 50 + 4)
                {
                    dmgcon(
                        victim.index,
                        StatusAilment::paralyzed,
                        rnd(element_power / 3 * 2 + 1));
                }
                if (rnd(20) < element_power / 50 + 4)
                {
                    dmgcon(
                        victim.index,
                        StatusAilment::confused,
                        rnd(element_power / 3 * 2 + 1));
                }
                if (rnd(20) < element_power / 50 + 4)
                {
                    dmgcon(
                        victim.index,
                        StatusAilment::poisoned,
                        rnd(element_power / 3 * 2 + 1));
                }
                if (rnd(20) < element_power / 50 + 4)
                {
                    dmgcon(
                        victim.index,
                        StatusAilment::sleep,
                        rnd(element_power / 3 * 2 + 1));
                }
            }
            if (element == 52)
            {
                if (rnd(3 + (victim.quality >= Quality::miracle) * 3) == 0)
                {
                    ++victim.paralyzed;
                }
            }
            if (element == 53)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::blinded,
                    rnd(element_power + 1));
            }
            if (element == 58)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::paralyzed,
                    rnd(element_power + 1));
            }
            if (element == 54)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::confused,
                    rnd(element_power + 1));
            }
            if (element == 57)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::confused,
                    rnd(element_power + 1));
            }
            if (element == 55)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::poisoned,
                    rnd(element_power + 1));
            }
            if (element == 61)
            {
                dmgcon(
                    victim.index,
                    StatusAilment::bleeding,
                    rnd(element_power + 1));
            }
            if (element == 62)
            {
                if (victim.index == 0)
                {
                    modify_ether_disease_stage(rnd(element_power + 1));
                }
            }
            if (element == 63)
            {
                if (victim.index == 0 || rnd(3) == 0)
                {
                    item_acid(victim);
                }
            }
        }
        if ((element == 50 || damage_source == -9) && victim.wet == 0)
        {
            item_fire(victim.index);
        }
        if (element == 51)
        {
            item_cold(victim.index);
        }
        if (victim.sleep != 0)
        {
            if (element != 54 && element != 58 && element != 59)
            {
                victim.sleep = 0;
                txt(i18n::s.get(
                    "core.locale.damage.sleep_is_disturbed", victim));
            }
        }
        if (attacker_is_player)
        {
            hostileaction(0, victim.index);
            game_data.chara_last_attacked_by_player = victim.index;
        }
        if (victim.index == 0)
        {
            if (Config::instance().sound)
            {
                if (Config::instance().heartbeat)
                {
                    int threshold = Config::instance().get<int>(
                        "core.screen.heartbeat_threshold");
                    if (victim.hp < victim.max_hp * (threshold * 0.01))
                    {
                        if (!CHECKPLAY(32))
                        {
                            snd("core.Heart1");

                            if (Config::instance().get<bool>(
                                    "core.android.vibrate"))
                            {
                                snail::android::vibrate_pulse();
                            }
                        }
                    }
                }
            }
        }
        if (victim.explodes())
        {
            if (rnd(3) == 0)
            {
                victim.will_explode_soon() = true;
                txt(i18n::s.get("core.locale.damage.explode_click"),
                    Message::color{ColorIndex::cyan});
            }
        }
        if (victim.splits())
        {
            if (game_data.proc_damage_events_flag != 1)
            {
                if (dmg_at_m141 > victim.max_hp / 20 || rnd(10) == 0)
                {
                    if (map_data.type != mdata_t::MapType::world_map)
                    {
                        if (chara_copy(victim) != -1)
                        {
                            txt(i18n::s.get(
                                "core.locale.damage.splits", victim));
                        }
                    }
                }
            }
        }
        if (victim.splits2())
        {
            if (game_data.proc_damage_events_flag != 1)
            {
                if (rnd(3) == 0)
                {
                    if (victim.confused == 0 && victim.dimmed == 0 &&
                        victim.poisoned == 0 && victim.paralyzed == 0 &&
                        victim.blind == 0)
                    {
                        if (map_data.type != mdata_t::MapType::world_map)
                        {
                            if (chara_copy(victim) != -1)
                            {
                                txt(i18n::s.get(
                                    "core.locale.damage.splits", victim));
                            }
                        }
                    }
                }
            }
        }
        if (victim.is_quick_tempered())
        {
            if (game_data.proc_damage_events_flag != 1)
            {
                if (victim.furious == 0)
                {
                    if (rnd(20) == 0)
                    {
                        if (is_in_fov(victim))
                        {
                            txt(i18n::s.get(
                                    "core.locale.damage.is_engulfed_in_fury",
                                    victim),
                                Message::color{ColorIndex::blue});
                        }
                        victim.furious += rnd(30) + 15;
                    }
                }
            }
        }
        if (attacker)
        {
            bool apply_hate = false;
            if (victim.relationship <= -3)
            {
                if (attacker->original_relationship > -3)
                {
                    if (victim.hate == 0 || rnd(4) == 0)
                    {
                        apply_hate = true;
                    }
                }
            }
            else if (attacker->original_relationship <= -3)
            {
                if (victim.hate == 0 || rnd(4) == 0)
                {
                    apply_hate = true;
                }
            }
            if (!attacker_is_player)
            {
                if (attacker->enemy_id == victim.index)
                {
                    if (rnd(3) == 0)
                    {
                        apply_hate = true;
                    }
                }
            }
            if (apply_hate)
            {
                if (victim.index != 0)
                {
                    victim.enemy_id = attacker->index;
                    if (victim.hate == 0)
                    {
                        victim.emotion_icon = 218;
                        victim.hate = 20;
                    }
                    else
                    {
                        victim.hate += 2;
                    }
                }
            }
        }
    }

    if (victim.hp < 0)
    {
        if (auto se = sound_id_for_element(element))
        {
            snd_at(*se, victim.position, false, false);
        }
        Message::instance().txtef(ColorIndex::red);
        if (attacker)
        {
            if (element)
            {
                if (victim.index >= 16 &&
                    game_data.proc_damage_events_flag == 2)
                {
                    Message::instance().continue_sentence();
                    txteledmg(1, attacker_is_player, victim.index, element);
                }
                else
                {
                    txteledmg(2, attacker_is_player, victim.index, element);
                }
            }
            else
            {
                int death_type = rnd(4);
                if (death_type == 0)
                {
                    if (victim.index >= 16 &&
                        game_data.proc_damage_events_flag == 2)
                    {
                        Message::instance().continue_sentence();
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.transformed_into_meat."
                            "active",
                            victim,
                            attacker_is_player));
                    }
                    else
                    {
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.transformed_into_meat."
                            "passive",
                            victim,
                            attacker_is_player));
                    }
                }
                if (death_type == 1)
                {
                    if (victim.index >= 16 &&
                        game_data.proc_damage_events_flag == 2)
                    {
                        Message::instance().continue_sentence();
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.destroyed.active",
                            victim,
                            attacker_is_player));
                    }
                    else
                    {
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.destroyed.passive",
                            victim,
                            attacker_is_player));
                    }
                }
                if (death_type == 2)
                {
                    if (victim.index >= 16 &&
                        game_data.proc_damage_events_flag == 2)
                    {
                        Message::instance().continue_sentence();
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.minced.active",
                            victim,
                            attacker_is_player));
                    }
                    else
                    {
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.minced.passive",
                            victim,
                            attacker_is_player));
                    }
                }
                if (death_type == 3)
                {
                    if (victim.index >= 16 &&
                        game_data.proc_damage_events_flag == 2)
                    {
                        Message::instance().continue_sentence();
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.killed.active",
                            victim,
                            attacker_is_player));
                    }
                    else
                    {
                        txt(i18n::s.get(
                            "core.locale.death_by.chara.killed.passive",
                            victim,
                            attacker_is_player));
                    }
                }
            }
            ndeathcause = i18n::s.get(
                "core.locale.death_by.chara.death_cause", *attacker);
        }
        else
        {
            if (damage_source == -6)
            {
                dmgheal_death_by_backpack(victim);
            }
            else
            {
                int death_kind = -damage_source;

                txt(i18n::s.get_enum_property(
                    "core.locale.death_by.other", "text", death_kind, victim));
                if (victim.index == 0)
                {
                    ndeathcause = i18n::s.get_enum_property(
                        "core.locale.death_by.other",
                        "death_cause",
                        death_kind);
                }
            }
        }
        if (damage_source == -9 || element == 50)
        {
            mef_add(
                victim.position.x,
                victim.position.y,
                5,
                24,
                rnd(10) + 5,
                100,
                damage_source);
        }
        if (element == 56)
        {
            if (attacker)
            {
                if (dmg_at_m141 > 0)
                {
                    heal_hp(
                        *attacker,
                        rnd(dmg_at_m141 * (200 + element_power) / 1000 + 5));
                }
            }
        }
        if (game_data.mount != victim.index || victim.index == 0)
        {
            cell_removechara(victim.position.x, victim.position.y);
        }

        Character::State new_state = dmgheal_set_death_status(victim);
        victim.set_state(new_state);

        if (victim.breaks_into_debris())
        {
            if (is_in_fov(victim))
            {
                x = victim.position.x;
                y = victim.position.y;
                snd_at("core.crush1", victim.position, false, false);
                animeblood(victim.index, 1, element);
            }
            spillfrag(victim.position.x, victim.position.y, 3);
        }
        else
        {
            sound_kill(victim.position);
            animeblood(victim.index, 0, element);
            spillblood(victim.position.x, victim.position.y, 4);
        }
        if (victim.index == 0)
        {
            ++game_data.death_count;
        }
        if (victim.index == game_data.chara_last_attacked_by_player)
        {
            game_data.chara_last_attacked_by_player = 0;
        }
        if (attacker)
        {
            if (!attacker_is_player)
            {
                chara_custom_talk(attacker->index, 103);
            }
            gained_exp = clamp(victim.level, 1, 200) *
                    clamp((victim.level + 1), 1, 200) *
                    clamp((victim.level + 2), 1, 200) / 20 +
                8;
            if (victim.level > attacker->level)
            {
                gained_exp /= 4;
            }
            if (victim.splits() || victim.splits2())
            {
                gained_exp /= 20;
            }
            attacker->experience += gained_exp;
            if (attacker_is_player)
            {
                game_data.sleep_experience += gained_exp;
            }
            attacker->hate = 0;
            if (attacker->index < 16)
            {
                attacker->enemy_id = 0;
                cdata.player().enemy_id = 0;
                game_data.chara_last_attacked_by_player = 0;
            }
        }
        if (victim.index != 0)
        {
            if (game_data.current_map != mdata_t::MapId::show_house)
            {
                if (game_data.current_map != mdata_t::MapId::the_void)
                {
                    if (victim.id == 2)
                    {
                        event_add(1);
                    }
                    if (victim.id == 141)
                    {
                        txt(i18n::s.get(
                                "core.locale.scenario.obtain_stone.fool"),
                            Message::color{ColorIndex::green});
                        snd("core.complete1");
                        game_data.quest_flags.magic_stone_of_fool = 1;
                    }
                    if (victim.id == 143)
                    {
                        txt(i18n::s.get(
                                "core.locale.scenario.obtain_stone.king"),
                            Message::color{ColorIndex::green});
                        snd("core.complete1");
                        game_data.quest_flags.magic_stone_of_king = 1;
                    }
                    if (victim.id == 144)
                    {
                        txt(i18n::s.get(
                                "core.locale.scenario.obtain_stone.sage"),
                            Message::color{ColorIndex::green});
                        snd("core.complete1");
                        game_data.quest_flags.magic_stone_of_sage = 1;
                    }
                    if (victim.id == 242)
                    {
                        if (game_data.quest_flags.novice_knight < 1000)
                        {
                            game_data.quest_flags.novice_knight = 2;
                            quest_update_journal_msg();
                        }
                    }
                    if (victim.id == 257)
                    {
                        if (game_data.quest_flags.pyramid_trial < 1000)
                        {
                            game_data.quest_flags.pyramid_trial = 1000;
                            quest_update_journal_msg();
                            txt(i18n::s.get("core.locale.quest.completed"));
                            snd("core.complete1");
                        }
                    }
                    if (victim.id == 300)
                    {
                        if (game_data.quest_flags.minotaur_king < 1000)
                        {
                            game_data.quest_flags.minotaur_king = 2;
                            quest_update_journal_msg();
                        }
                    }
                    if (victim.id == 318)
                    {
                        event_add(27, victim.position.x, victim.position.y);
                    }
                    if (victim.id == 319)
                    {
                        ++game_data.quest_flags.kill_count_of_little_sister;
                        txt(i18n::s.get(
                                "core.locale.talk.unique.strange_scientist."
                                "saved_"
                                "count",
                                game_data.quest_flags
                                    .save_count_of_little_sister,
                                game_data.quest_flags
                                    .kill_count_of_little_sister),
                            Message::color{ColorIndex::red});
                    }
                    if (game_data.current_dungeon_level ==
                            area_data[game_data.current_map].deepest_level ||
                        game_data.current_map == mdata_t::MapId::the_void)
                    {
                        if (area_data[game_data.current_map]
                                    .has_been_conquered == victim.index &&
                            victim.is_lord_of_dungeon() == 1)
                        {
                            event_add(5);
                        }
                    }
                    if (victim.id == 331)
                    {
                        if (rnd(4) == 0)
                        {
                            event_add(28, victim.position.x, victim.position.y);
                        }
                    }
                    quest_check();
                }
                else if (game_data.current_map == mdata_t::MapId::the_void)
                {
                    if (area_data[game_data.current_map].has_been_conquered ==
                            victim.index &&
                        victim.is_lord_of_dungeon() == 1)
                    {
                        event_add(5);
                    }
                }
            }
        }
        if (victim.index != 0)
        {
            ++npcmemory(0, victim.id);
            chara_custom_talk(victim.index, 102);
            if (victim.index < 16)
            {
                txt(i18n::s.get("core.locale.damage.you_feel_sad"));
            }
        }
        if (victim.state() == Character::State::empty)
        {
            // Exclude town residents because they occupy character slots even
            // if they are dead.
            modify_crowd_density(victim.index, -1);
        }
        if (game_data.mount)
        {
            if (victim.index == game_data.mount)
            {
                txt(i18n::s.get("core.locale.damage.get_off_corpse", victim));
                ride_end();
            }
        }
        check_kill(damage_source, victim.index);
        catitem = 0;
        rollanatomy = 0;
        if (rnd(60) == 0)
        {
            rollanatomy = 1;
        }
        if (attacker)
        {
            if (attacker->id == 260)
            {
                catitem = attacker->index;
            }
            if (int(std::sqrt(sdata(161, attacker->index))) > rnd(150))
            {
                rollanatomy = 1;
            }
            chara_gain_skill_exp(*attacker, 161, 10 + rollanatomy * 4);
        }
        rc = victim.index;
        character_drops_item();
        if (game_data.current_map == mdata_t::MapId::pet_arena)
        {
            if (rnd(5) == 0)
            {
                snd("core.cheer");
            }
        }
        if (victim.is_death_master() == 1)
        {
            txt(i18n::s.get("core.locale.damage.death_word_breaks"));
            for (int chara_index = 0; chara_index < ELONA_MAX_CHARACTERS;
                 ++chara_index)
            {
                if (cdata[chara_index].state() != Character::State::alive)
                {
                    continue;
                }
                for (int buff_index = 0; buff_index < 16; ++buff_index)
                {
                    if (cdata[chara_index].buffs[buff_index].id == 0)
                    {
                        break;
                    }
                    if (cdata[chara_index].buffs[buff_index].id == 16)
                    {
                        buff_delete(cdata[chara_index], buff_index);
                        --buff_index;
                        continue;
                    }
                }
            }
        }
        if (attacker_is_player)
        {
            if (game_data.catches_god_signal)
            {
                if (rnd(20) == 0)
                {
                    txtgod(cdata.player().god_id, 9);
                }
            }
        }

        lua::lua->get_event_manager().trigger(lua::CharacterKilledEvent(
            victim, dmg_at_m141, element, damage_source));

        end_dmghp(victim);

        return 0;
    }
    end_dmghp(victim);
    return 1;
}



void heal_hp(Character& cc, int delta)
{
    cc.hp += delta;
    if (cc.hp > cc.max_hp)
    {
        cc.hp = cc.max_hp;
    }
}



void damage_mp(Character& cc, int delta)
{
    cc.mp -= delta;
    if (cc.mp < -999999)
    {
        cc.mp = -999999;
    }
    if (cc.mp < 0)
    {
        gain_mana_capacity_experience(cc.index);
        auto damage = -cc.mp * 400 / (100 + sdata(164, cc.index) * 10);
        if (cc.index == 0)
        {
            if (trait(156) == 1)
            {
                damage /= 2;
            }
        }
        else
        {
            damage /= 5;
            if (damage < 10)
                return;
        }
        txt(i18n::s.get("core.locale.damage.magic_reaction_hurts", cc));
        damage_hp(cc, damage, -2);
    }
}



void heal_mp(Character& cc, int delta)
{
    cc.mp += delta;
    if (cc.mp > cc.max_mp)
    {
        cc.mp = cc.max_mp;
    }
}



bool action_sp(Character& cc, int sp)
{
    if (cc.index != 0 || debug::voldemort)
        return true;

    damage_sp(cc, sp);
    return cc.sp >= 50 || cc.sp >= rnd(75);
}



void damage_sp(Character& cc, int delta)
{
    if (cc.index != 0 || debug::voldemort)
        return;

    if (cc.sp >= -100)
    {
        cc.sp -= delta;
    }
}



void heal_sp(Character& cc, int delta)
{
    cc.sp += delta;
    if (cc.sp > cc.max_sp)
    {
        cc.sp = cc.max_sp;
    }
}



void damage_insanity(Character& cc, int delta)
{
    if (cc.quality >= Quality::miracle)
        return;

    int resistance = std::max(sdata(54, cc.index) / 50, 1);
    if (resistance > 10)
        return;

    delta /= resistance;
    if (cc.index < 16)
    {
        if (trait(166))
        {
            delta -= rnd(4);
        }
    }
    delta = std::max(delta, 0);
    cc.insanity += delta;
    if (rnd(10) == 0 || rnd(delta + 1) > 5 || rnd(cc.insanity + 1) > 50)
    {
        dmgcon(cc.index, StatusAilment::insane, 100);
    }
}



void heal_insanity(Character& cc, int delta)
{
    cc.insanity -= delta;
    if (cc.insanity < 0)
    {
        cc.insanity = 0;
    }
}



} // namespace elona
