#include "equipment.hpp"
#include "adventurer.hpp"
#include "calc.hpp"
#include "character.hpp"
#include "character_status.hpp"
#include "class.hpp"
#include "data/types/type_item.hpp"
#include "draw.hpp"
#include "enchantment.hpp"
#include "globals.hpp"
#include "i18n.hpp"
#include "item.hpp"
#include "itemgen.hpp"
#include "random.hpp"
#include "variables.hpp"



namespace elona
{

namespace
{

snail::Color _get_element_color(int element)
{
    switch (element)
    {
    case 50: return {150, 0, 0};
    case 51: return {0, 0, 150};
    case 52: return {150, 150, 0};
    case 59: return {150, 0, 150};
    case 53: return {100, 80, 80};
    case 55: return {0, 150, 0};
    case 60: return {150, 100, 100};
    case 57: return {50, 100, 150};
    case 58: return {100, 150, 50};
    case 54: return {150, 100, 50};
    case 56: return {150, 50, 0};
    default: return {0, 0, 0};
    }
}



void draw_additional_item_info_resistance(const Item& equip, int x, int y)
{
    for (const auto& enc : equip.enchantments)
    {
        if (enc.id == 0)
            break;
        if (enc.id / 10000 != 2)
            continue;

        const auto power_level = std::abs(enc.power / 2 / 50) + 1;
        const auto element = enc.id % 10000;
        const auto color = _get_element_color(element);
        if (jp)
        {
            const auto s =
                power_level > 5 ? u8"+"s : std::to_string(power_level);
            const auto glyph = enc.power >= 0 ? u8"●" : u8"▼";
            mes(x + (element - 50) * 20, y, glyph, color);
            mes(x + (element - 50) * 20 + 5, y + 1, s, color);
            mes(x + (element - 50) * 20 + 4, y, s, {255, 255, 255});
        }
        else
        {
            mes(x + (element - 50) * 20 + 5,
                y,
                std::to_string(power_level),
                {80, 60, 40});
            mes(x + (element - 50) * 20 + 4,
                y,
                std::to_string(power_level),
                color);
        }
    }
}



void draw_additional_item_info_maintenance_and_ailment(
    const Item& equip,
    int x,
    int y)
{
    for (int i = 0; i < 10; ++i)
    {
        if (enchantment_find(equip, 60000 + 10 + i))
        {
            mes(x - 100 + i * 20, y, jp ? u8"●" : "#", {0, 100, 100});
        }
        else
        {
            mes(x - 100 + i * 20, y, "-", {0, 100, 100});
        }
    }
    for (int i = 0; i < 6; ++i)
    {
        if (enchantment_find(equip, 23 + i))
        {
            mes(x + 100 + i * 20, y, jp ? u8"●" : "#", {100, 32, 0});
        }
        else
        {
            mes(x + 100 + i * 20, y, "-", {100, 32, 0});
        }
    }
}



optional<const char*> enchantment_type2icon_id(const Enchantment& enc)
{
    switch (enc.id)
    {
    case 10002: return "enchantment_icon_increases_life";
    case 10003: return "enchantment_icon_increases_mana";
    case 10010:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_strength"
                                   : "enchantment_icon_decreases_strength";
    case 10011:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_consititution"
                                   : "enchantment_icon_decreases_consititution";
    case 10012:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_dexterity"
                                   : "enchantment_icon_decreases_dexterity";
    case 10013:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_perception"
                                   : "enchantment_icon_decreases_perception";
    case 10014:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_learning"
                                   : "enchantment_icon_decreases_learning";
    case 10015:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_will"
                                   : "enchantment_icon_decreases_will";
    case 10016:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_magic"
                                   : "enchantment_icon_decreases_magic";
    case 10017:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_charisma"
                                   : "enchantment_icon_decreases_charisma";
    case 10018:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_speed"
                                   : "enchantment_icon_decreases_speed";
    case 10019:
        return enc.power / 50 >= 0 ? "enchantment_icon_increases_luck"
                                   : "enchantment_icon_decreases_luck";
    case 20050:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_fire"
            : "enchantment_icon_weakens_resistance_to_fire";
    case 20051:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_cold"
            : "enchantment_icon_weakens_resistance_to_cold";
    case 20052:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_lightning"
            : "enchantment_icon_weakens_resistance_to_lightning";
    case 20053:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_darkness"
            : "enchantment_icon_weakens_resistance_to_darkness";
    case 20054:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_mind"
            : "enchantment_icon_weakens_resistance_to_mind";
    case 20055:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_poison"
            : "enchantment_icon_weakens_resistance_to_poison";
    case 20056:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_nether"
            : "enchantment_icon_weakens_resistance_to_nether";
    case 20057:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_sound"
            : "enchantment_icon_weakens_resistance_to_sound";
    case 20058:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_nerve"
            : "enchantment_icon_weakens_resistance_to_nerve";
    case 20059:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_chaos"
            : "enchantment_icon_weakens_resistance_to_chaos";
    case 20060:
        return enc.power / 50 >= 0
            ? "enchantment_icon_grants_resistance_to_magic"
            : "enchantment_icon_weakens_resistance_to_magic";
    case 60010: return "enchantment_icon_maintains_strength";
    case 60011: return "enchantment_icon_maintains_consititution";
    case 60012: return "enchantment_icon_maintains_dexterity";
    case 60013: return "enchantment_icon_maintains_perception";
    case 60014: return "enchantment_icon_maintains_learning";
    case 60015: return "enchantment_icon_maintains_will";
    case 60016: return "enchantment_icon_maintains_magic";
    case 60017: return "enchantment_icon_maintains_charisma";
    case 60018: return "enchantment_icon_maintains_speed";
    case 60019: return "enchantment_icon_maintains_luck";
    case 70050: return "enchantment_icon_deals_fire_damage";
    case 70051: return "enchantment_icon_deals_cold_damage";
    case 70052: return "enchantment_icon_deals_lightning_damage";
    case 70053: return "enchantment_icon_deals_darkness_damage";
    case 70054: return "enchantment_icon_deals_mind_damage";
    case 70055: return "enchantment_icon_deals_poison_damage";
    case 70056: return "enchantment_icon_deals_nether_damage";
    case 70057: return "enchantment_icon_deals_sound_damage";
    case 70058: return "enchantment_icon_deals_nerve_damage";
    case 70059: return "enchantment_icon_deals_chaos_damage";
    case 70060: return "enchantment_icon_deals_magic_damage";
    case 70061: return "enchantment_icon_deals_cut_damage";
    case 90000: return "enchantment_icon_can_be_loaded_with_rapid_ammo";
    case 90001: return "enchantment_icon_can_be_loaded_with_explosive_ammo";
    case 90002: return "enchantment_icon_can_be_loaded_with_piercing_ammo";
    case 90003: return "enchantment_icon_can_be_loaded_with_magic_ammo";
    case 90004: return "enchantment_icon_can_be_loaded_with_time_stop_ammo";
    case 90005: return "enchantment_icon_can_be_loaded_with_burst_ammo";
    case 21: return "enchantment_icon_causes_random_teleport";
    case 22: return "enchantment_icon_prevents_you_from_teleporting";
    case 23: return "enchantment_icon_negates_blindness";
    case 24: return "enchantment_icon_negates_paralysis";
    case 25: return "enchantment_icon_negates_confusion";
    case 26: return "enchantment_icon_negates_fear";
    case 27: return "enchantment_icon_negates_sleep";
    case 28: return "enchantment_icon_negates_poison";
    case 29: return "enchantment_icon_speeds_up_travel_progress";
    case 30: return "enchantment_icon_protects_you_from_etherwind";
    case 31: return "enchantment_icon_negates_bad_weather";
    case 32: return "enchantment_icon_floats_you";
    case 33: return "enchantment_icon_protects_you_from_mutation";
    case 34: return "enchantment_icon_enhances_spells";
    case 35: return "enchantment_icon_allows_you_to_see_invisible_creatures";
    case 36: return "enchantment_icon_absorbs_stamina";
    case 37: return "enchantment_icon_brings_an_end";
    case 38: return "enchantment_icon_absorbs_mana";
    case 39: return "enchantment_icon_gives_absolute_piercing_attack";
    case 40: return "enchantment_icon_stops_time";
    case 41: return "enchantment_icon_protects_you_from_thieves";
    case 42: return "enchantment_icon_allows_you_to_digest_rotten_food";
    case 43: return "enchantment_icon_protects_you_from_cursing_words";
    case 44: return "enchantment_icon_increases_critical_hits";
    case 45: return "enchantment_icon_sucks_blood";
    case 46: return "enchantment_icon_disturbs_your_growth";
    case 47: return "enchantment_icon_attracts_monsters";
    case 48: return "enchantment_icon_prevents_aliens_from_entering_your_body";
    case 49: return "enchantment_icon_increases_quality_of_reward";
    case 50: return "enchantment_icon_increases_extra_melee_attack";
    case 51: return "enchantment_icon_increases_extra_ranged_attack";
    case 52: return "enchantment_icon_decreases_physical_damage";
    case 53: return "enchantment_icon_nullifies_damage";
    case 54: return "enchantment_icon_deals_cut_damage_to_the_attacker";
    case 55: return "enchantment_icon_diminishes_bleeding";
    case 56: return "enchantment_icon_catches_signals";
    case 57: return "enchantment_icon_inflicts_massive_damage_to_dragons";
    case 58: return "enchantment_icon_inflicts_massive_damage_to_undeads";
    case 59: return "enchantment_icon_reveals_religion";
    case 60: return "enchantment_icon_makes_audience_drunk";
    case 61: return "enchantment_icon_inflicts_massive_damage_to_gods";
    default: return none;
    }
}



void draw_additional_item_info_all_attributes(const Item& equip, int x, int y)
{
    if (equip.is_fireproof())
    {
        mes(x - 100, y, jp ? u8"●" : "#", {100, 32, 0});
    }
    else
    {
        mes(x - 100, y, "-", {100, 32, 0});
    }
    if (equip.is_acidproof())
    {
        mes(x - 80, y, jp ? u8"●" : "#", {100, 32, 0});
    }
    else
    {
        mes(x - 80, y, "-", {100, 32, 0});
    }

    int i = 0;
    for (const auto& enc : equip.enchantments)
    {
        if (enc.id == 0)
            break;

        if (const auto icon_id = enchantment_type2icon_id(enc))
        {
            draw(*icon_id, x - 60 + i * 20, y);
            ++i;
        }
    }
}

} // namespace



int i_at_m66 = 0;



void draw_additional_item_info_label(int x, int y)
{
    switch (g_show_additional_item_info)
    {
    case AdditionalItemInfo::none:
    case AdditionalItemInfo::_size: break;
    case AdditionalItemInfo::resistance:
        for (int i = 0; i < 11; ++i)
        {
            mes(x + 20 * i,
                y,
                i18n::s.get_enum("core.locale.ui.equip.resist", i));
        }
        break;
    case AdditionalItemInfo::maintenance_and_ailment:
        for (int i = 0; i < 10; ++i)
        {
            mes(x - 100 + 20 * i,
                y,
                i18n::s.get_enum("core.locale.ui.equip.maintenance", i));
        }
        for (int i = 0; i < 6; ++i)
        {
            mes(x + 100 + 20 * i,
                y,
                i18n::s.get_enum("core.locale.ui.equip.ailment", i));
        }
        break;
    case AdditionalItemInfo::all_attributes:
        mes(x - 100, y, i18n::s.get("core.locale.ui.equip.fireproof"));
        mes(x - 80, y, i18n::s.get("core.locale.ui.equip.acidproof"));
        break;
    }
}



void draw_additional_item_info(const Item& equip, int x, int y)
{
    if (equip.identification_state != IdentifyState::completely_identified)
        return;

    switch (g_show_additional_item_info)
    {
    case AdditionalItemInfo::none:
    case AdditionalItemInfo::_size: break;
    case AdditionalItemInfo::resistance:
        draw_additional_item_info_resistance(equip, x, y);
        break;
    case AdditionalItemInfo::maintenance_and_ailment:
        draw_additional_item_info_maintenance_and_ailment(equip, x, y);
        break;
    case AdditionalItemInfo::all_attributes:
        draw_additional_item_info_all_attributes(equip, x, y);
        break;
    }
}



std::string cut_item_name_for_additional_info(
    const std::string& name,
    size_t adjustment)
{
    size_t width;
    switch (g_show_additional_item_info)
    {
    case AdditionalItemInfo::none:
    case AdditionalItemInfo::_size: return name;
    case AdditionalItemInfo::resistance: width = 24; break;
    case AdditionalItemInfo::maintenance_and_ailment: width = 14; break;
    case AdditionalItemInfo::all_attributes: width = 14; break;
    }
    width -= adjustment;

    return strutil::take_by_width(name, width);
}



int eqweaponlight()
{
    if (rnd(2))
    {
        return 10003;
    }
    if (rnd(2))
    {
        return 10009;
    }
    return 10004;
}



int eqweaponheavy()
{
    if (rnd(3) == 0)
    {
        return 10001;
    }
    if (rnd(3) == 0)
    {
        return 10010;
    }
    if (rnd(3) == 0)
    {
        return 10008;
    }
    return 10005;
}



void eqrandweaponmage()
{
    if (rnd(2))
    {
        eqweapon1(0) = 10006;
        eqweapon1(1) = 2;
    }
    else
    {
        eqweapon1(0) = 10003;
        eqweapon1(1) = 2;
    }
}

void wear_most_valuable_equipment_for_all_body_parts()
{
    for (const auto& cnt : items(rc))
    {
        ci = cnt;
        if (inv[cnt].number() == 0 || inv[cnt].body_part != 0)
        {
            continue;
        }
        wear_most_valuable_equipment();
    }
}



void wear_most_valuable_equipment()
{
    int eqdup = 0;
    elona_vector1<int> bodylist;
    i = iequiploc(ci);
    if (i != 0)
    {
        eqdup = 0;
        for (int j = 0; j < 30; ++j)
        {
            if (cdata[rc].body_parts[j] / 10000 == i)
            {
                bodylist(eqdup) = j + 100;
                ++eqdup;
            }
        }
        if (eqdup == 0)
        {
            return;
        }
        for (int cnt = 0, cnt_end = (eqdup); cnt < cnt_end; ++cnt)
        {
            body = bodylist(cnt);
            i = cdata[rc].body_parts[body - 100] % 10000;
            if (i == 0)
            {
                equip_item(rc);
                break;
            }
            --i;
            f = 0;
            if (inv[ci].value >= inv[i].value)
            {
                f = 1;
            }
            if (eqdup > cnt + 1)
            {
                if (cdata[rc].body_parts[bodylist(cnt + 1) - 100] % 10000 == 0)
                {
                    f = 0;
                }
                else if (
                    inv[i].value >=
                    inv[cdata[rc].body_parts[bodylist(cnt + 1) - 100] % 10000 -
                        1]
                        .value)
                {
                    f = 0;
                }
            }
            if (f == 1)
            {
                int cibk = ci;
                unequip_item(rc);
                ci = cibk;
                equip_item(rc);
                break;
            }
        }
    }
}



void supply_new_equipment()
{
    int haveweapon = 0;
    int mustequip = 0;
    haveweapon = 0;
    for (int cnt = 0; cnt < 100; ++cnt)
    {
        f = 0;
        for (int cnt = 0; cnt < 4; ++cnt)
        {
            ci = get_random_inv(rc);
            if (inv[ci].number() == 0)
            {
                f = 1;
                break;
            }
            if (inv[ci].body_part != 0)
            {
                continue;
            }
            if (inv[ci].is_quest_target())
            {
                continue;
            }
            if (inv[ci].number() != 0)
            {
                inv[ci].remove();
                f = 1;
                break;
            }
        }
        if (f == 0)
        {
            ci = invhead + invrange - 1;
        }
        if (cdata[rc].character_role == 13)
        {
            flt(cdata[rc].level, Quality::great);
        }
        else
        {
            flt(cdata[rc].level, calcfixlv(Quality::bad));
        }
        mustequip = 0;
        for (int cnt = 0; cnt < 30; ++cnt)
        {
            p = cdata[rc].body_parts[cnt] / 10000;
            if (p == 0)
            {
                break;
            }
            if (cdata[rc].body_parts[cnt] % 10000 != 0)
            {
                if (p == 5)
                {
                    if (haveweapon == 0)
                    {
                        if (the_item_db
                                [inv[cdata[rc].body_parts[cnt] % 10000 - 1].id]
                                    ->category == 10000)
                        {
                            haveweapon = 1;
                        }
                    }
                }
                continue;
            }
            if (p == 5)
            {
                if (haveweapon == 0)
                {
                    flttypemajor = 10000;
                    mustequip = 1;
                    break;
                }
            }
            if (p == 1)
            {
                flttypemajor = 12000;
                mustequip = 1;
                break;
            }
            if (p == 4)
            {
                flttypemajor = 16000;
                mustequip = 1;
                break;
            }
            if (p == 10)
            {
                flttypeminor = 24001;
                mustequip = 1;
                break;
            }
            if (p == 11)
            {
                flttypeminor = 25001;
                mustequip = 1;
                break;
            }
        }
        if (mustequip == 0)
        {
            break;
        }
        int stat = itemcreate(rc, 0, -1, -1, 0);
        if (stat == 0)
        {
            break;
        }
        inv[ci].identification_state = IdentifyState::completely_identified;
        if (inv[ci].quality >= Quality::miracle)
        {
            if (the_item_db[inv[ci].id]->category < 50000)
            {
                if (cdata[rc].character_role == 13)
                {
                    addnews(1, rc, 0, itemname(ci));
                }
            }
        }
        wear_most_valuable_equipment();
        if (cdata[rc].character_role != 13)
        {
            if (rnd(3))
            {
                break;
            }
        }
    }
}



void supply_initial_equipments()
{
    elona_vector1<int> eqhelm;
    elona_vector1<int> eqshield;
    elona_vector1<int> eqweapon2;
    elona_vector1<int> eqarmor;
    elona_vector1<int> eqglove;
    elona_vector1<int> eqboots;
    elona_vector1<int> eqcloack;
    elona_vector1<int> eqgirdle;
    elona_vector1<int> eqring2;
    elona_vector1<int> eqamulet1;
    elona_vector1<int> eqamulet2;
    int fixeq = 0;
    int probeq = 0;
    int eqtwowield = 0;
    if (cdatan(2, rc) == u8"core.mutant"s)
    {
        for (int cnt = 0, cnt_end = cnt + clamp(cdata[rc].level / 3, 0, 12);
             cnt < cnt_end;
             ++cnt)
        {
            gain_new_body_part(cdata[rc]);
        }
    }

    eqhelm.clear();
    eqweapon1.clear();
    eqshield.clear();
    eqrange.clear();
    eqammo.clear();
    eqweapon2.clear();
    eqarmor.clear();
    eqglove.clear();
    eqboots.clear();
    eqcloack.clear();
    eqgirdle.clear();
    eqring1.clear();
    eqring2.clear();
    eqamulet1.clear();
    eqamulet2.clear();

    if (mode == 1)
    {
        fixeq = 0;
        probeq = 10;
    }
    else if (cdata[rc].quality <= Quality::good)
    {
        probeq = 3;
        fixeq = 0;
    }
    else if (cdata[rc].quality == Quality::great)
    {
        probeq = 6;
        fixeq = 0;
    }
    else if (cdata[rc].quality == Quality::miracle)
    {
        probeq = 8;
        fixeq = 1;
    }
    else
    {
        probeq = 10;
        fixeq = 1;
    }
    if (cequipment == 1)
    {
        generatemoney(rc);
        eqweapon1(0) = eqweaponheavy();
        eqweapon1(1) = 2;
        eqshield(0) = 14003;
        eqshield(1) = 1;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqhelm(0) = 12001;
            eqhelm(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqgirdle(0) = 19001;
            eqgirdle(1) = 1;
        }
        eqrange(0) = 24030;
        eqrange(1) = 1;
    }
    if (cequipment == 8)
    {
        generatemoney(rc);
        eqweapon1(0) = 232;
        eqweapon1(1) = 3;
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqgirdle(0) = 19001;
            eqgirdle(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqcloack(0) = 20001;
            eqcloack(1) = 2;
        }
        eqrange(0) = 24030;
        eqrange(1) = 1;
        eqtwohand = 1;
    }
    if (cequipment == 7)
    {
        generatemoney(rc);
        eqweapon1(0) = eqweaponlight();
        eqweapon1(1) = 2;
        eqweapon2(0) = eqweaponlight();
        eqweapon2(1) = 2;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqhelm(0) = 12001;
            eqhelm(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqgirdle(0) = 19001;
            eqgirdle(1) = 1;
        }
        eqrange(0) = 24030;
        eqrange(1) = 1;
        eqtwowield = 1;
    }
    if (cequipment == 6)
    {
        generatemoney(rc);
        eqweapon1(0) = 10004;
        eqweapon1(1) = 1;
        if (rnd(10) < probeq)
        {
            eqshield(0) = 14003;
            eqshield(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqhelm(0) = 12001;
            eqhelm(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
    }
    if (cequipment == 3)
    {
        generatemoney(rc);
        eqweapon1(0) = 10002;
        eqweapon1(1) = 1;
        eqrange(0) = 24001;
        eqrange(1) = 1;
        eqammo(0) = 25001;
        eqammo(1) = 1;
        eqcloack(0) = 20001;
        eqcloack(1) = 1;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqglove(0) = 22001;
            eqglove(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
    }
    if (cequipment == 2)
    {
        generatemoney(rc);
        eqrandweaponmage();
        eqamulet1(0) = 34001;
        eqamulet1(1) = 1;
        eqring1(0) = 32001;
        eqring1(1) = 2;
        eqring2(0) = 32001;
        eqring2(1) = 1;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16003;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqcloack(0) = 20001;
            eqcloack(1) = 1;
        }
    }
    if (cequipment == 4)
    {
        generatemoney(rc);
        eqweapon1(0) = 10002;
        eqweapon1(1) = 1;
        if (rnd(4) != 0)
        {
            eqrange(0) = 24020;
            eqrange(1) = 2;
            eqammo(0) = 25020;
            eqammo(1) = 1;
        }
        else
        {
            eqrange(0) = 24021;
            eqrange(1) = 2;
            eqammo(0) = 25030;
            eqammo(1) = 1;
        }
        eqcloack(0) = 20001;
        eqcloack(1) = 1;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqglove(0) = 22001;
            eqglove(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqboots(0) = 18001;
            eqboots(1) = 1;
        }
    }
    if (cequipment == 5)
    {
        generatemoney(rc);
        eqrandweaponmage();
        eqamulet1(0) = 34001;
        eqamulet1(1) = 1;
        eqring1(0) = 32001;
        eqring1(1) = 2;
        eqring2(0) = 32001;
        eqring2(1) = 1;
        if (rnd(10) < probeq)
        {
            eqarmor(0) = 16001;
            eqarmor(1) = 1;
        }
        if (rnd(10) < probeq)
        {
            eqcloack(0) = 20001;
            eqcloack(1) = 1;
        }
    }
    if (cdata[rc].quality >= Quality::miracle)
    {
        for (int cnt = 0; cnt < 2; ++cnt)
        {
            if (rnd(2) == 0)
            {
                switch (rnd(20))
                {
                case 0: eqhelm(1) = 3; break;
                case 1: eqweapon1(1) = 3; break;
                case 2: eqshield(1) = 3; break;
                case 3: eqrange(1) = 3; break;
                case 4: eqammo(1) = 3; break;
                case 5: eqweapon2(1) = 3; break;
                case 6: eqarmor(1) = 3; break;
                case 7: eqglove(1) = 3; break;
                case 8: eqboots(1) = 3; break;
                case 9: eqcloack(1) = 3; break;
                case 10: eqgirdle(1) = 3; break;
                case 11: eqring1(1) = 3; break;
                case 12: eqring2(1) = 3; break;
                case 13: eqamulet1(1) = 3; break;
                case 14: eqamulet2(1) = 3; break;
                default: break;
                }
            }
            if (rnd(2) == 0)
            {
                --cnt;
                continue;
            }
        }
    }
    if (cspecialeq)
    {
        dbmode = 4;
        dbid = cdata[rc].id;
        access_character_info();
    }
    if (eqtwohand)
    {
        eqshield = 0;
    }
    if (cdata[rc].id == 9)
    {
        if (rnd(120) == 0)
        {
            eqamulet1 = 705;
        }
    }
    if (cdata[rc].id == 274)
    {
        if (rnd(100) == 0)
        {
            eqrange = 718;
        }
    }
    if (cdata[rc].id == 163)
    {
        if (rnd(200) == 0)
        {
            eqrange = 716;
        }
    }
    if (cdata[rc].id == 186 || cdata[rc].id == 187 || cdata[rc].id == 188)
    {
        if (rnd(800) == 0)
        {
            eqgirdle = 728;
        }
    }
    if (cdata[rc].id == 317)
    {
        if (rnd(150) == 0)
        {
            eqweapon1 = 719;
        }
    }
    if (cdata[rc].id == 309 || cdata[rc].id == 310 || cdata[rc].id == 311)
    {
        if (rnd(600) == 0)
        {
            eqamulet1 = 723;
        }
    }
    if (cdata[rc].id == 304)
    {
        if (rnd(250) == 0)
        {
            eqamulet1 = 722;
        }
    }
    for (int i = 0; i < 30; ++i)
    {
        p = cdata[rc].body_parts[i] / 10000;
        if (p == 0)
        {
            break;
        }
        if (p == 2)
        {
            if (eqamulet1)
            {
                if (eqamulet1 >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqamulet1(1))));
                    flttypeminor = eqamulet1;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqamulet1;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqamulet1 = 0;
                continue;
            }
            if (eqamulet2)
            {
                if (eqamulet2 >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqamulet2(1))));
                    flttypeminor = eqamulet2;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqamulet2;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqamulet2 = 0;
                continue;
            }
        }
        if (p == 6)
        {
            if (eqring1)
            {
                if (eqring1 >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqring1(1))));
                    flttypeminor = eqring1;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqring1;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqring1 = 0;
                continue;
            }
            if (eqring2)
            {
                if (eqring2 >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqring2(1))));
                    flttypeminor = eqring2;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqring2;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqring2 = 0;
                continue;
            }
        }
        if (p == 3)
        {
            if (eqcloack)
            {
                if (eqcloack >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqcloack(1))));
                    flttypeminor = eqcloack;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqcloack;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqcloack = 0;
                continue;
            }
            continue;
        }
        if (p == 8)
        {
            if (eqgirdle)
            {
                if (eqgirdle >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqgirdle(1))));
                    flttypeminor = eqgirdle;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqgirdle;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqgirdle = 0;
                continue;
            }
            continue;
        }
        if (p == 1)
        {
            if (eqhelm)
            {
                if (eqhelm >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqhelm(1))));
                    flttypeminor = eqhelm;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqhelm;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqhelm = 0;
                continue;
            }
            continue;
        }
        if (p == 4)
        {
            if (eqarmor)
            {
                if (eqarmor >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqarmor(1))));
                    flttypeminor = eqarmor;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqarmor;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqarmor = 0;
                continue;
            }
            continue;
        }
        if (p == 7)
        {
            if (eqglove)
            {
                if (eqglove >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqglove(1))));
                    flttypeminor = eqglove;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqglove;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqglove = 0;
                continue;
            }
            continue;
        }
        if (p == 9)
        {
            if (eqboots)
            {
                if (eqboots >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqboots(1))));
                    flttypeminor = eqboots;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqboots;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqboots = 0;
                continue;
            }
            continue;
        }
        if (p == 5)
        {
            if (eqmultiweapon)
            {
                for (int cnt = 0; cnt < 15; ++cnt)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(2 + fixeq)));
                    itemcreate(rc, eqmultiweapon, -1, -1, 0);
                    if (inv[ci].weight > 1500)
                    {
                        if (cnt < 14)
                        {
                            inv[ci].remove();
                            continue;
                        }
                    }
                    break;
                }
                body = 100 + i;
                equip_item(rc);
                eqweapon1 = 0;
                continue;
            }
            if (eqweapon1)
            {
                if (eqweapon1 >= 10000)
                {
                    for (int cnt = 0; cnt < 15; ++cnt)
                    {
                        flt(calcobjlv(cdata[rc].level),
                            calcfixlv(
                                static_cast<Quality>(fixeq + eqweapon1(1))));
                        flttypeminor = eqweapon1;
                        dbid = -1;
                        itemcreate(rc, dbid, -1, -1, 0);
                        if (eqtwohand)
                        {
                            if (inv[ci].weight < 4000)
                            {
                                if (cnt < 14)
                                {
                                    inv[ci].remove();
                                    continue;
                                }
                            }
                        }
                        if (eqtwowield)
                        {
                            if (inv[ci].weight > 1500)
                            {
                                if (cnt < 14)
                                {
                                    inv[ci].remove();
                                    continue;
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    flt();
                    dbid = eqweapon1;
                    itemcreate(rc, dbid, -1, -1, 0);
                }
                body = 100 + i;
                equip_item(rc);
                eqweapon1 = 0;
                continue;
            }
            if (eqweapon2)
            {
                if (eqweapon2 >= 10000)
                {
                    for (int cnt = 0; cnt < 15; ++cnt)
                    {
                        flt(calcobjlv(cdata[rc].level),
                            calcfixlv(
                                static_cast<Quality>(fixeq + eqweapon2(1))));
                        flttypeminor = eqweapon2;
                        dbid = -1;
                        itemcreate(rc, dbid, -1, -1, 0);
                        if (eqtwowield)
                        {
                            if (inv[ci].weight > 1500)
                            {
                                if (cnt < 14)
                                {
                                    inv[ci].remove();
                                    continue;
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    flt();
                    dbid = eqweapon2;
                    itemcreate(rc, dbid, -1, -1, 0);
                }
                eqweapon2 = 0;
                body = 100 + i;
                equip_item(rc);
                continue;
            }
            if (eqshield)
            {
                if (eqshield >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqshield(1))));
                    flttypeminor = eqshield;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqshield;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqshield = 0;
                continue;
            }
            continue;
        }
        if (p == 10)
        {
            if (eqrange)
            {
                if (eqrange >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqrange(1))));
                    flttypeminor = eqrange;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqrange;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqrange = 0;
                continue;
            }
            continue;
        }
        if (p == 11)
        {
            if (eqammo)
            {
                if (eqammo >= 10000)
                {
                    flt(calcobjlv(cdata[rc].level),
                        calcfixlv(static_cast<Quality>(fixeq + eqammo(1))));
                    flttypeminor = eqammo;
                    dbid = -1;
                }
                else
                {
                    flt();
                    dbid = eqammo;
                }
                itemcreate(rc, dbid, -1, -1, 0);
                body = 100 + i;
                equip_item(rc);
                eqammo = 0;
                continue;
            }
            continue;
        }
    }
    if (cdata[rc].id == 326)
    {
        if (rnd(150) == 0)
        {
            flt();
            itemcreate(rc, 707, -1, -1, 0);
        }
        else
        {
            flt(calcobjlv(cdata[rc].level), calcfixlv());
            flttypeminor = 60005;
            itemcreate(rc, 0, -1, -1, 0);
        }
    }
    if (cdata[rc].id == 351 || cdata[rc].id == 352)
    {
        for (int cnt = 0; cnt < 6; ++cnt)
        {
            flt();
            nostack = 1;
            int stat = itemcreate(rc, 772, -1, -1, 0);
            if (stat != 0)
            {
                inv[ci].modify_number(rnd(4));
                if (rnd(2))
                {
                    inv[ci].param3 = -1;
                    inv[ci].image = 336;
                }
            }
        }
    }
    eqtwohand = 0;
    eqtwowield = 0;
    eqmultiweapon = 0;
}

} // namespace elona
