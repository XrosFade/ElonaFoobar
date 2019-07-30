#include "item.hpp"
#include <iostream>
#include <type_traits>
#include "../util/strutil.hpp"
#include "ability.hpp"
#include "activity.hpp"
#include "area.hpp"
#include "audio.hpp"
#include "blending.hpp"
#include "building.hpp"
#include "character.hpp"
#include "crafting.hpp"
#include "data/types/type_fish.hpp"
#include "data/types/type_item.hpp"
#include "data/types/type_item_material.hpp"
#include "elona.hpp"
#include "enums.hpp"
#include "food.hpp"
#include "fov.hpp"
#include "i18n.hpp"
#include "lua_env/lua_env.hpp"
#include "map.hpp"
#include "mef.hpp"
#include "message.hpp"
#include "random.hpp"
#include "text.hpp"
#include "ui.hpp"
#include "variables.hpp"

static std::string lang(const std::string& a, const std::string& b)
{
    return jp ? a : b;
}


namespace elona
{


Inventory inv;

int ci_at_m138 = 0;
int p_at_m138 = 0;
int max_at_m138 = 0;
int ti_at_m138 = 0;
elona_vector1<int> list_at_m138;
int f_at_m138 = 0;
int a_at_m138 = 0;


Item::Item()
    : enchantments(15)
{
}



void Item::clear()
{
    copy({}, *this);
}



bool Item::almost_equals(const Item& other, bool ignore_position)
{
    return true
        // && number == other.number
        && value == other.value && image == other.image &&
        id == other.id
        // && quality == other.quality
        && (ignore_position || position == other.position) &&
        weight == other.weight &&
        identification_state == other.identification_state &&
        count == other.count && dice_x == other.dice_x &&
        dice_y == other.dice_y && damage_bonus == other.damage_bonus &&
        hit_bonus == other.hit_bonus && dv == other.dv && pv == other.pv &&
        skill == other.skill && curse_state == other.curse_state &&
        body_part == other.body_part && function == other.function &&
        enhancement == other.enhancement && own_state == other.own_state &&
        color == other.color && subname == other.subname &&
        material == other.material && param1 == other.param1 &&
        param2 == other.param2 && param3 == other.param3 &&
        param4 == other.param4 &&
        difficulty_of_identification == other.difficulty_of_identification
        // && turn == other.turn
        && _flags == other._flags &&
        range::equal(enchantments, other.enchantments);
}

Inventory::Inventory()
    : storage(ELONA_MAX_ITEMS)
{
    for (size_t i = 0; i < storage.size(); ++i)
    {
        storage[i].index = static_cast<int>(i);
    }
}



int f_at_m53 = 0;
int f_at_m54 = 0;
elona_vector1<int> p_at_m57;
std::string s_;
int a_ = 0;
int skip_ = 0;


range::iota<int> items(int owner)
{
    const auto tmp = inv_getheader(owner);
    return {tmp.first, tmp.first + tmp.second};
}



/**
 * Tries to find an item in the player's inventory, the ground, or both. Returns
 * the item's index, or -1 if not found.
 *
 * Valid values of @ref matcher_type:
 *   0: By category
 *   1: By skill
 *   2: By subcategory
 *   3: By ID
 *
 * Valid values of @ref inventory_type:
 *   -1: On ground
 *    0: Both player's inventory and on ground
 *    1: Player's inventory
 */
int item_find(int matcher, int matcher_type, ItemFindLocation location_type)
{
    elona_vector1<int> p_at_m52;
    p_at_m52(0) = -1;
    p_at_m52(1) = -1;
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        int invhead;
        int invrange;
        if (cnt == 0)
        {
            if (location_type == ItemFindLocation::player_inventory)
            {
                continue;
            }
            const auto pair = inv_getheader(-1);
            invhead = pair.first;
            invrange = pair.second;
        }
        else
        {
            if (location_type == ItemFindLocation::ground)
            {
                continue;
            }
            const auto pair = inv_getheader(0);
            invhead = pair.first;
            invrange = pair.second;
        }
        p_at_m52(2) = cnt;
        for (int cnt = invhead, cnt_end = cnt + (invrange); cnt < cnt_end;
             ++cnt)
        {
            if (inv[cnt].number() == 0)
            {
                continue;
            }
            if (p_at_m52(2) == 0)
            {
                if (inv[cnt].position.x != cdata.player().position.x ||
                    inv[cnt].position.y != cdata.player().position.y)
                {
                    continue;
                }
            }
            if (matcher_type == 0)
            {
                if (the_item_db[inv[cnt].id]->category == matcher)
                {
                    p_at_m52 = cnt;
                }
            }
            if (matcher_type == 1)
            {
                if (inv[cnt].skill == matcher)
                {
                    if (p_at_m52(1) < inv[cnt].param1)
                    {
                        p_at_m52(0) = cnt;
                        p_at_m52(1) = inv[cnt].param1;
                    }
                }
            }
            if (matcher_type == 2)
            {
                if (the_item_db[inv[cnt].id]->subcategory == matcher)
                {
                    p_at_m52 = cnt;
                }
            }
            if (matcher_type == 3)
            {
                if (inv[cnt].id == matcher)
                {
                    p_at_m52 = cnt;
                }
            }
        }
    }
    return p_at_m52;
}



std::vector<int> itemlist(int owner, int id)
{
    std::vector<int> ret;
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].id == id)
        {
            ret.push_back(cnt);
        }
    }
    return ret;
}



int itemusingfind(int ci, bool disallow_pc)
{
    for (auto&& cnt : cdata.all())
    {
        if (cnt.state() != Character::State::alive)
        {
            continue;
        }
        if (cnt.continuous_action &&
            cnt.continuous_action.type != ContinuousAction::Type::sex &&
            cnt.continuous_action.turn > 0 && cnt.continuous_action.item == ci)
        {
            if (!disallow_pc || cnt.index != 0)
            {
                return cnt.index;
            }
        }
    }
    return -1; // Not found
}



int itemfind(int inventory_id, int matcher, int matcher_type)
{
    f_at_m54 = -1;
    if (matcher_type == 0)
    {
        for (const auto& cnt : items(inventory_id))
        {
            if (inv[cnt].number() == 0)
            {
                continue;
            }
            if (inv[cnt].id == matcher)
            {
                f_at_m54 = cnt;
                break;
            }
        }
        return f_at_m54;
    }
    else
    {
        for (const auto& cnt : items(inventory_id))
        {
            if (inv[cnt].number() == 0)
            {
                continue;
            }
            if (the_item_db[inv[cnt].id]->subcategory == matcher)
            {
                f_at_m54 = cnt;
                break;
            }
        }
        return f_at_m54;
    }
}



int mapitemfind(int x, int y, int id)
{
    for (const auto& cnt : items(-1))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].id == id && inv[cnt].position == Position{x, y})
        {
            return cnt;
        }
    }
    return -1; // Not found
}



void cell_refresh(int x, int y)
{
    int p_at_m55 = 0;
    elona_vector1<int> n_at_m55;
    int cnt2_at_m55 = 0;
    int i_at_m55 = 0;
    if (mode == 6 || mode == 9)
    {
        return;
    }
    if (x < 0 || y < 0 || x >= map_data.width || y >= map_data.height)
    {
        return;
    }

    p_at_m55 = 0;
    cell_data.at(x, y).item_appearances_actual = 0;
    cell_data.at(x, y).light = 0;
    for (const auto& cnt : items(-1))
    {
        if (inv[cnt].number() > 0)
        {
            if (inv[cnt].position.x == x && inv[cnt].position.y == y)
            {
                floorstack(p_at_m55) = cnt;
                ++p_at_m55;
                wpoke(
                    cell_data.at(x, y).item_appearances_actual,
                    0,
                    inv[cnt].image);
                wpoke(
                    cell_data.at(x, y).item_appearances_actual,
                    2,
                    inv[cnt].color);
                if (ilight(inv[cnt].id) != 0)
                {
                    cell_data.at(x, y).light = ilight(inv[cnt].id);
                }
            }
        }
    }
    if (p_at_m55 > 3)
    {
        wpoke(
            cell_data.at(x, y).item_appearances_actual,
            0,
            363); // Item bag chip ID
        wpoke(
            cell_data.at(x, y).item_appearances_actual,
            2,
            static_cast<int>(ColorIndex::none));
    }
    else if (p_at_m55 > 1)
    {
        n_at_m55(2) = 0;
        for (int cnt = 0, cnt_end = (p_at_m55); cnt < cnt_end; ++cnt)
        {
            cnt2_at_m55 = cnt;
            i_at_m55 = -1;
            for (int cnt = 0, cnt_end = (p_at_m55); cnt < cnt_end; ++cnt)
            {
                if (cnt2_at_m55 == 1)
                {
                    if (cnt == n_at_m55(0))
                    {
                        continue;
                    }
                }
                if (cnt2_at_m55 == 2)
                {
                    if (cnt == n_at_m55(0) || cnt == n_at_m55(1))
                    {
                        continue;
                    }
                }
                if (inv[floorstack(cnt)].turn > i_at_m55)
                {
                    n_at_m55(cnt2_at_m55) = cnt;
                    i_at_m55 = inv[floorstack(cnt)].turn;
                }
            }
        }
        cell_data.at(x, y).item_appearances_actual =
            floorstack(n_at_m55(0)) - ELONA_ITEM_ON_GROUND_INDEX;
        cell_data.at(x, y).item_appearances_actual +=
            (floorstack(n_at_m55(1)) - ELONA_ITEM_ON_GROUND_INDEX) * 1000;
        if (p_at_m55 > 2)
        {
            cell_data.at(x, y).item_appearances_actual +=
                (floorstack(n_at_m55(2)) - ELONA_ITEM_ON_GROUND_INDEX) *
                1000000;
        }
        else
        {
            cell_data.at(x, y).item_appearances_actual += 999000000;
        }
        cell_data.at(x, y).item_appearances_actual *= -1;
    }
}



void itemturn(int ci)
{
    if (game_data.item_turns < 0)
    {
        game_data.item_turns = 0;
    }
    ++game_data.item_turns;
    inv[ci].turn = game_data.item_turns;
}


void item_copy(int a, int b)
{
    if (a < 0 || b < 0)
        return;

    bool was_empty = inv[b].number() == 0;

    if (was_empty && inv[a].number() > 0)
    {
        // Clean up any stale handles that may have been left over from an item
        // in the same index being removed.
        lua::lua->get_handle_manager().remove_item_handle_run_callbacks(inv[b]);
    }

    Item::copy(inv[a], inv[b]);

    if (was_empty && inv[b].number() != 0)
    {
        lua::lua->get_handle_manager().create_item_handle_run_callbacks(inv[b]);
    }
    else if (!was_empty && inv[b].number() == 0)
    {
        inv[b].remove();
    }
}



void item_exchange(int a, int b)
{
    Item tmp;
    Item::copy(inv[a], tmp);
    Item::copy(inv[b], inv[a]);
    Item::copy(tmp, inv[b]);

    lua::lua->get_handle_manager().swap_handles<Item>(inv[b], inv[a]);
}



void Item::remove()
{
    number_ = 0;
}

void item_delete(int ci)
{
    inv[ci].remove();

    inv[ci].clear();
    // Restore "index".
    inv[ci].index = ci;
}



void item_refresh(Item& i)
{
    if (i.number() <= 0)
    {
        i.remove();
    }
    if (i.index >= ELONA_ITEM_ON_GROUND_INDEX && mode != 6)
    {
        // Refresh the cell the item is on if it's on the ground.
        cell_refresh(i.position.x, i.position.y);
    }
    else if (i.index < 200)
    {
        // Refresh the player's burden state if the item is in their
        // inventory.
        refresh_burden_state();
    }
}



void Item::modify_number(int delta)
{
    this->set_number(this->number_ + delta);
}



void Item::set_number(int number_)
{
    bool item_was_empty = this->number_ <= 0;

    if (item_was_empty && number_ > 0)
    {
        // Clean up any stale handles that may have been left over from an item
        // in the same index being removed.
        lua::lua->get_handle_manager().remove_item_handle_run_callbacks(*this);
    }

    this->number_ = std::max(number_, 0);
    item_refresh(*this);

    bool created_new = item_was_empty && this->number_ > 0;
    if (created_new)
    {
        lua::lua->get_handle_manager().create_item_handle_run_callbacks(*this);
    }
}



int item_separate(int ci)
{
    if (inv[ci].number() <= 1)
        return ci;

    int ti = inv_getfreeid(inv_getowner(ci));
    if (ti == -1)
    {
        ti = inv_getfreeid(-1);
        if (ti == -1)
        {
            inv[ci].set_number(1);
            txt(i18n::s.get("core.locale.item.something_falls_and_disappears"));
            return ci;
        }
    }

    item_copy(ci, ti);
    inv[ti].set_number(inv[ci].number() - 1);
    inv[ci].set_number(1);

    if (inv_getowner(ti) == -1 && mode != 6)
    {
        if (inv_getowner(ci) != -1)
        {
            inv[ci].position = cdata[inv_getowner(ci)].position;
        }
        inv[ti].position = inv[ci].position;
        itemturn(ti);
        cell_refresh(inv[ti].position.x, inv[ti].position.y);
        if (inv_getowner(ci) != -1)
        {
            txt(i18n::s.get("core.locale.item.something_falls_from_backpack"));
        }
        refresh_burden_state();
    }

    return ti;
}



bool chara_unequip(int ci)
{
    if (inv[ci].body_part == 0)
        return false;

    int body_part = inv[ci].body_part;
    int owner = inv_getowner(ci);
    if (owner == -1)
        return false;

    cdata[owner].body_parts[body_part - 100] =
        cdata[owner].body_parts[body_part - 100] / 10000 * 10000;
    inv[ci].body_part = 0;
    return true;
}


IdentifyState item_identify(Item& ci, IdentifyState level)
{
    if (level == IdentifyState::almost_identified &&
        the_item_db[ci.id]->category >= 50000)
    {
        level = IdentifyState::completely_identified;
    }
    if (ci.identification_state >= level)
    {
        idtresult = IdentifyState::unidentified;
        return idtresult;
    }
    ci.identification_state = level;
    if (ci.identification_state >= IdentifyState::partly_identified)
    {
        itemmemory(0, ci.id) = 1;
    }
    idtresult = level;
    return idtresult;
}


IdentifyState item_identify(Item& ci, int power)
{
    return item_identify(
        ci,
        power >= ci.difficulty_of_identification
            ? IdentifyState::completely_identified
            : IdentifyState::unidentified);
}


void item_checkknown(int ci)
{
    if (inv[ci].identification_state == IdentifyState::completely_identified)
    {
        inv[ci].identification_state = IdentifyState::completely_identified;
    }
    if (itemmemory(0, inv[ci].id) &&
        inv[ci].identification_state == IdentifyState::unidentified)
    {
        item_identify(inv[ci], IdentifyState::partly_identified);
    }
}



void itemname_additional_info(int item_index)
{
    if (inv[item_index].id == 578)
    {
        s_ += i18n::s.get_enum(
            "core.locale.item.kitty_bank_rank", inv[item_index].param2);
    }
    if (inv[item_index].id == 617)
    {
        s_ += lang(
            ""s +
                i18n::s.get_enum(
                    "core.locale.item.bait_rank", inv[item_index].param1),
            u8" <"s +
                i18n::s.get_enum(
                    "core.locale.item.bait_rank", inv[item_index].param1) +
                u8">"s);
    }
    if (inv[item_index].id == 687)
    {
        if (jp)
        {
            if (inv[item_index].param2 != 0)
            {
                s_ += u8"解読済みの"s;
            }
        }
        if (inv[item_index].identification_state ==
            IdentifyState::completely_identified)
        {
            s_ += lang(
                u8"《"s +
                    i18n::s.get_enum(
                        "core.locale.item.ancient_book_title",
                        inv[item_index].param1) +
                    u8"》という題名の"s,
                u8" titled <"s +
                    i18n::s.get_enum(
                        "core.locale.item.ancient_book_title",
                        inv[item_index].param1) +
                    u8">"s);
        }
    }
    if (inv[item_index].id == 783)
    {
        if (inv[item_index].param1 == 0)
        {
            s_ += lang(u8"もう使えない"s, u8" which cannot be used anymore"s);
        }
        else if (inv[item_index].subname == 0)
        {
            s_ += lang(u8"カスタム"s, ""s);
        }
        else
        {
            s_ += lang(
                u8"《"s + rpname(inv[item_index].subname) + u8"》の"s,
                u8" of <"s + rpname(inv[item_index].subname) + u8">"s);
        }
    }
    if (a_ == 55000)
    {
        if (inv[item_index].id == 563)
        {
            s_ += lang(
                u8"《"s +
                    i18n::s.get_m(
                        "locale.ability",
                        the_ability_db
                            .get_id_from_legacy(inv[item_index].param1)
                            ->get(),
                        "name") +
                    u8"》という題名の"s,
                u8" titled <Art of "s +
                    i18n::s.get_m(
                        "locale.ability",
                        the_ability_db
                            .get_id_from_legacy(inv[item_index].param1)
                            ->get(),
                        "name") +
                    u8">"s);
        }
        else if (inv[item_index].id == 668)
        {
            s_ += lang(u8"第"s, u8" of Rachel No."s) + inv[item_index].param2 +
                lang(u8"巻目の"s, ""s);
        }
        else if (inv[item_index].id == 24)
        {
            s_ += lang(
                u8"《"s + booktitle(inv[item_index].param1) +
                    u8"》という題名の"s,
                u8" titled <"s + booktitle(inv[item_index].param1) + u8">"s);
        }
    }
    if (a_ == 60002)
    {
        if (inv[item_index].param1 != 0)
        {
            s_ += lang(
                god_name(inv[item_index].param1) + u8"の"s,
                u8" <"s + god_name(inv[item_index].param1) + u8">"s);
        }
    }
    if (a_ == 57000)
    {
        if (inv[item_index].param1 != 0)
        {
            if (inv[item_index].param2 != 0)
            {
                skip_ = 1;
                if (inv[item_index].id == 618)
                {
                    s_ =
                        s_ +
                        foodname(
                            inv[item_index].param1 / 1000,
                            i18n::s.get_m(
                                "locale.fish",
                                the_fish_db
                                    .get_id_from_legacy(inv[item_index].subname)
                                    ->get(),
                                "name"),
                            inv[item_index].param2,
                            inv[item_index].subname);
                }
                else
                {
                    s_ = s_ +
                        foodname(
                             inv[item_index].param1 / 1000,
                             ioriginalnameref(inv[item_index].id),
                             inv[item_index].param2,
                             inv[item_index].subname);
                }
                return;
            }
        }
        if (inv[item_index].own_state == 4)
        {
            s_ += lang(""s, u8" grown "s) +
                i18n::s.get_enum(
                    "core.locale.ui.weight", inv[item_index].subname) +
                lang(u8"育った"s, ""s);
        }
    }
    if (inv[item_index].subname != 0)
    {
        if (inv[item_index].id == 618 || inv[item_index].id == 619)
        {
            if (inv[item_index].subname < 0 || inv[item_index].subname >= 100)
            {
                s_ += u8"/bugged/"s;
                return;
            }
            s_ += i18n::s.get_m(
                "locale.fish",
                the_fish_db.get_id_from_legacy(inv[item_index].subname)->get(),
                "name");
        }
        else if (
            a_ == 57000 || a_ == 62000 || inv[item_index].id == 503 ||
            inv[item_index].id == 504 || inv[item_index].id == 575 ||
            inv[item_index].id == 574)
        {
            if (inv[item_index].subname < 0 || inv[item_index].subname >= 800)
            {
                s_ += u8"/bugged/"s;
                return;
            }
            if (inv[item_index].own_state != 4)
            {
                s_ += lang(""s, u8" of "s) +
                    chara_refstr(inv[item_index].subname, 2);
                if (jp)
                {
                    s_ += u8"の"s;
                }
            }
        }
        if (a_ == 60000)
        {
            if (jp)
            {
                if (inv[item_index].subname >= 12)
                {
                    inv[item_index].subname = 0;
                }
                else
                {
                    s_ += i18n::s.get_enum(
                        "core.locale.ui.furniture", inv[item_index].subname);
                }
            }
        }
        if (inv[item_index].id == 344)
        {
            s_ += lang(""s, u8" of "s) +
                i18n::s.get_enum(
                    "core.locale.ui.home", inv[item_index].param1) +
                lang(u8"の"s, ""s);
        }
        if (inv[item_index].id == 615)
        {
            s_ += lang(
                ""s + inv[item_index].subname + u8"goldの"s,
                u8" <"s + inv[item_index].subname + u8" gp>"s);
        }
        if (inv[item_index].id == 704)
        {
            if (inv[item_index].subname < 0 || inv[item_index].subname >= 800)
            {
                s_ += u8"/bugged/"s;
                return;
            }
            s_ += lang(
                ""s + chara_refstr(inv[item_index].subname, 2) + u8"の"s,
                u8" of "s + chara_refstr(inv[item_index].subname, 2));
        }
    }
    if (inv[item_index].id == 672)
    {
        if (inv[item_index].param1 == 169)
        {
            s_ += lang(u8"善人の"s, u8" of saint"s);
        }
        if (inv[item_index].param1 == 162)
        {
            s_ += lang(u8"悪人の"s, u8" of wicked"s);
        }
        if (inv[item_index].param1 == 163)
        {
            s_ += lang(u8"エヘカトルの"s, u8" of Ehekatl"s);
        }
        if (inv[item_index].param1 == 164)
        {
            s_ += lang(u8"オパートスの"s, u8" of Opatos"s);
        }
        if (inv[item_index].param1 == 165)
        {
            s_ += lang(u8"イツパロトルの"s, u8" of Itzpalt"s);
        }
        if (inv[item_index].param1 == 166)
        {
            s_ += lang(u8"ジュアの"s, u8" of Jure"s);
        }
    }
}



std::string itemname(int item_index, int number, int skip_article)
{
    elona_vector1<int> iqiality_;
    int num2_ = 0;
    std::string s2_;
    std::string s3_;
    int alpha_ = 0;
    std::string s4_;
    elona_vector1<std::string> buf_;
    if (inv[item_index].id >= maxitemid - 2 ||
        size_t(inv[item_index].id) > ioriginalnameref.size())
    {
        return i18n::s.get("core.locale.item.unknown_item");
    }
    if (inv[item_index].quality >= Quality::godly)
    {
        iqiality_(item_index) = 5;
    }
    item_checkknown(item_index);
    if (number == 0)
    {
        num2_ = inv[item_index].number();
    }
    else
    {
        num2_ = number;
    }
    a_ = the_item_db[inv[item_index].id]->category;
    if (jp)
    {
        if (num2_ > 1)
        {
            s2_ = u8"個の"s;
            if (a_ == 16000)
            {
                s2_ = u8"着の"s;
            }
            if (a_ == 54000 || a_ == 55000)
            {
                if (inv[item_index].id == 783)
                {
                    s2_ = u8"枚の"s;
                }
                else
                {
                    s2_ = u8"冊の"s;
                }
            }
            if (a_ == 10000)
            {
                s2_ = u8"本の"s;
            }
            if (a_ == 52000)
            {
                s2_ = u8"服の"s;
            }
            if (a_ == 53000)
            {
                s2_ = u8"巻の"s;
            }
            if (a_ == 22000 || a_ == 18000)
            {
                s2_ = u8"対の"s;
            }
            if (a_ == 68000 || a_ == 69000 || inv[item_index].id == 622 ||
                inv[item_index].id == 724 || inv[item_index].id == 730)
            {
                s2_ = u8"枚の"s;
            }
            if (inv[item_index].id == 618)
            {
                s2_ = u8"匹の"s;
            }
            s_ = ""s + num2_ + s2_;
        }
        else
        {
            s_ = "";
        }
        if (inv[item_index].identification_state ==
            IdentifyState::completely_identified)
        {
            switch (inv[item_index].curse_state)
            {
            case CurseState::doomed:
                s_ += i18n::s.get("core.locale.ui.curse_state.doomed");
                break;
            case CurseState::cursed:
                s_ += i18n::s.get("core.locale.ui.curse_state.cursed");
                break;
            case CurseState::none: break;
            case CurseState::blessed:
                s_ += i18n::s.get("core.locale.ui.curse_state.blessed");
                break;
            }
        }
    }
    else
    {
        s_ = "";
        if (inv[item_index].identification_state ==
            IdentifyState::completely_identified)
        {
            switch (inv[item_index].curse_state)
            {
            case CurseState::doomed:
                s_ = i18n::s.get("core.locale.ui.curse_state.doomed") + u8" "s;
                break;
            case CurseState::cursed:
                s_ = i18n::s.get("core.locale.ui.curse_state.cursed") + u8" "s;
                break;
            case CurseState::none: break;
            case CurseState::blessed:
                s_ = i18n::s.get("core.locale.ui.curse_state.blessed") + u8" "s;
                break;
            }
        }
        if (irandomname(inv[item_index].id) == 1 &&
            inv[item_index].identification_state == IdentifyState::unidentified)
        {
            s2_ = "";
        }
        else
        {
            s2_ = ""s + ioriginalnameref2(inv[item_index].id);
            if (strutil::contains(
                    ioriginalnameref(inv[item_index].id), u8"with"))
            {
                s3_ = u8"with"s;
            }
            else
            {
                s3_ = u8"of"s;
            }
            if (inv[item_index].identification_state !=
                    IdentifyState::unidentified &&
                s2_ == ""s)
            {
                if (inv[item_index].weight < 0)
                {
                    s2_ = u8"cargo"s;
                }
                if (a_ == 22000 || a_ == 18000)
                {
                    s2_ = u8"pair"s;
                }
            }
            if (a_ == 57000 && inv[item_index].param1 != 0 &&
                inv[item_index].param2 != 0)
            {
                s2_ = u8"dish"s;
            }
        }
        if (s2_ != ""s)
        {
            if (num2_ > 1)
            {
                s_ = ""s + num2_ + u8" "s + s_ + s2_ + u8"s "s + s3_ + u8" "s;
            }
            else
            {
                s_ = s_ + s2_ + u8" "s + s3_ + u8" "s;
            }
        }
        else if (num2_ > 1)
        {
            s_ = ""s + num2_ + u8" "s + s_;
        }
    }
    if (inv[item_index].material == 35 && inv[item_index].param3 < 0)
    {
        if (jp)
        {
            s_ += u8"腐った"s;
        }
        else
        {
            s_ += u8"rotten "s;
        }
    }
    if (en)
    {
        if (a_ == 57000 && inv[item_index].param1 != 0 &&
            inv[item_index].param2 != 0)
        {
            skip_ = 1;
        }
        if (inv[item_index].subname != 0 && a_ == 60000)
        {
            if (inv[item_index].subname >= 12)
            {
                inv[item_index].subname = 0;
            }
            else
            {
                s_ += i18n::s.get_enum(
                          "core.locale.ui.furniture", inv[item_index].subname) +
                    u8" "s;
            }
        }
        if (inv[item_index].id == 687 && inv[item_index].param2 != 0)
        {
            s_ += u8"undecoded "s;
        }
        if (inv[item_index].id == 783 && inv[item_index].subname == 0)
        {
            s_ += u8"custom "s;
        }
    }
    if (inv[item_index].id == 630)
    {
        s_ += ""s +
            i18n::s.get_m(
                "locale.item_material",
                the_item_material_db
                    .get_id_from_legacy(inv[item_index].material)
                    ->get(),
                "name") +
            lang(u8"製の"s, u8" "s);
    }
    if (jp)
    {
        itemname_additional_info(item_index);
    }
    if (a_ == 60000 && inv[item_index].material != 0)
    {
        if (jp)
        {
            s_ += ""s +
                i18n::s.get_m(
                    "locale.item_material",
                    the_item_material_db
                        .get_id_from_legacy(inv[item_index].material)
                        ->get(),
                    "name") +
                u8"細工の"s;
        }
        else
        {
            s_ += ""s +
                i18n::s.get_m(
                    "locale.item_material",
                    the_item_material_db
                        .get_id_from_legacy(inv[item_index].material)
                        ->get(),
                    "name") +
                u8"work "s;
        }
    }
    if (inv[item_index].id == 729)
    {
        s_ += i18n::s.get_enum(
                  "core.locale.item.gift_rank", inv[item_index].param4) +
            i18n::space_if_needed();
    }
    if (skip_ == 1)
    {
        goto label_0313_internal;
    }
    alpha_ = 0;
    if (inv[item_index].identification_state ==
            IdentifyState::completely_identified &&
        a_ < 50000)
    {
        if (inv[item_index].is_eternal_force())
        {
            alpha_ = 1;
            s_ += lang(u8"エターナルフォース"s, u8"eternal force"s) +
                i18n::space_if_needed();
        }
        else
        {
            if (inv[item_index].subname >= 10000)
            {
                if (inv[item_index].subname < 20000)
                {
                    if (jp)
                    {
                        s_ += egoname(inv[item_index].subname - 10000) +
                            i18n::space_if_needed();
                    }
                }
                else if (inv[item_index].subname < 40000)
                {
                    s_ += egominorn(inv[item_index].subname - 20000) +
                        i18n::space_if_needed();
                }
            }
            if (inv[item_index].quality != Quality::special)
            {
                if (inv[item_index].quality >= Quality::miracle)
                {
                    s_ += i18n::s.get_m(
                              "locale.item_material",
                              the_item_material_db
                                  .get_id_from_legacy(inv[item_index].material)
                                  ->get(),
                              "alias") +
                        i18n::space_if_needed();
                }
                else
                {
                    s_ += i18n::s.get_m(
                              "locale.item_material",
                              the_item_material_db
                                  .get_id_from_legacy(inv[item_index].material)
                                  ->get(),
                              "name") +
                        i18n::space_if_needed();
                    if (jp)
                    {
                        if (/* TODO is_katakana */ false)
                        {
                            alpha_ = 1;
                        }
                        else
                        {
                            s_ += u8"の"s;
                        }
                    }
                }
            }
        }
    }
    if (inv[item_index].identification_state == IdentifyState::unidentified)
    {
        s_ += iknownnameref(inv[item_index].id);
    }
    else if (
        inv[item_index].identification_state !=
        IdentifyState::completely_identified)
    {
        if (inv[item_index].quality < Quality::miracle || a_ >= 50000)
        {
            s_ += ioriginalnameref(inv[item_index].id);
        }
        else
        {
            s_ += iknownnameref(inv[item_index].id);
        }
    }
    else if (
        inv[item_index].quality == Quality::special ||
        inv[item_index].is_precious())
    {
        if (jp)
        {
            s_ = u8"★"s + s_ + ioriginalnameref(inv[item_index].id);
        }
        else
        {
            s_ += ioriginalnameref(inv[item_index].id);
        }
    }
    else
    {
        if (inv[item_index].quality >= Quality::miracle && jp)
        {
            s_ = u8"☆"s + s_;
        }
        if (alpha_ == 1 && jp)
        {
            s_ += ialphanameref(inv[item_index].id);
        }
        else
        {
            s_ += ioriginalnameref(inv[item_index].id);
        }
        if (en && a_ < 50000 && inv[item_index].subname >= 10000 &&
            inv[item_index].subname < 20000)
        {
            s_ += u8" "s + egoname((inv[item_index].subname - 10000));
        }
        if (inv[item_index].subname >= 40000)
        {
            randomize(inv[item_index].subname - 40000);
            if (inv[item_index].quality == Quality::miracle)
            {
                s_ += i18n::space_if_needed() +
                    i18n::s.get(
                        "core.locale.item.miracle_paren",
                        random_title(RandomTitleType::weapon));
            }
            else
            {
                s_ += i18n::space_if_needed() +
                    i18n::s.get(
                        "core.locale.item.godly_paren",
                        random_title(RandomTitleType::weapon));
            }
            randomize();
        }
    }
label_0313_internal:
    if (en)
    {
        if (skip_article == 0)
        {
            if (inv[item_index].identification_state ==
                    IdentifyState::completely_identified &&
                (inv[item_index].quality >= Quality::miracle && a_ < 50000))
            {
                s_ = u8"the "s + s_;
            }
            else if (num2_ == 1)
            {
                s4_ = strmid(s_, 0, 1);
                if (s4_ == u8"a"s || s4_ == u8"o"s || s4_ == u8"i"s ||
                    s4_ == u8"u"s || s4_ == u8"e"s)
                {
                    s_ = u8"an "s + s_;
                }
                else
                {
                    s_ = u8"a "s + s_;
                }
            }
        }
        if (s2_ == ""s && inv[item_index].id != 618 && num2_ > 1)
        {
            s_ += u8"s"s;
        }
        itemname_additional_info(item_index);
    }
    if (inv[item_index].identification_state ==
        IdentifyState::completely_identified)
    {
        if (inv[item_index].enhancement != 0)
        {
            s_ += ""s + cnvfix(inv[item_index].enhancement) + u8" "s;
        }
        if (inv[item_index].has_charge())
        {
            s_ +=
                i18n::s.get("core.locale.item.charges", inv[item_index].count);
        }
        if (inv[item_index].dice_x != 0 || inv[item_index].hit_bonus != 0 ||
            inv[item_index].damage_bonus != 0)
        {
            s_ += u8" ("s;
            if (inv[item_index].dice_x != 0)
            {
                s_ += ""s + inv[item_index].dice_x + u8"d"s +
                    inv[item_index].dice_y;
                if (inv[item_index].damage_bonus != 0)
                {
                    if (inv[item_index].damage_bonus > 0)
                    {
                        s_ += u8"+"s + inv[item_index].damage_bonus;
                    }
                    else
                    {
                        s_ += ""s + inv[item_index].damage_bonus;
                    }
                }
                s_ += u8")"s;
                if (inv[item_index].hit_bonus != 0)
                {
                    s_ += u8"("s + inv[item_index].hit_bonus + u8")"s;
                }
            }
            else
            {
                s_ += ""s + inv[item_index].hit_bonus + u8","s +
                    inv[item_index].damage_bonus + u8")"s;
            }
        }
        if (inv[item_index].dv != 0 || inv[item_index].pv != 0)
        {
            s_ += u8" ["s + inv[item_index].dv + u8","s + inv[item_index].pv +
                u8"]"s;
        }
    }
    if (en && (inv[item_index].id == 284 || inv[item_index].id == 283))
    {
        s_ += u8"(Lost property)"s;
    }
    if (inv[item_index].id == 342 && inv[item_index].count != 0)
    {
        s_ += lang(
            u8"("s +
                i18n::s.get_enum(
                    "core.locale.item.bait_rank", inv[item_index].param4) +
                u8"残り"s + inv[item_index].count + u8"匹)"s,
            u8"("s + inv[item_index].count + u8" "s +
                i18n::s.get_enum(
                    "core.locale.item.bait_rank", inv[item_index].param4) +
                u8")"s);
    }
    if (inv[item_index].id == 685)
    {
        if (inv[item_index].subname == 0)
        {
            s_ += lang(u8" Lv"s, u8" Level "s) + inv[item_index].param2 +
                lang(u8" (空)"s, u8"(Empty)"s);
        }
        else
        {
            s_ += u8" ("s + chara_refstr(inv[item_index].subname, 2) + u8")"s;
        }
    }
    if (inv[item_index].id == 734)
    {
        s_ += lang(u8" Lv"s, u8" Level "s) + inv[item_index].param2;
    }
    if (inv[item_index].identification_state ==
            IdentifyState::almost_identified &&
        a_ < 50000)
    {
        s_ += u8" ("s +
            cnven(i18n::s.get_enum(
                u8"core.locale.ui.quality",
                static_cast<int>(inv[item_index].quality))) +
            u8")"s;
        if (jp)
        {
            s_ += u8"["s +
                i18n::s.get_m(
                    "locale.item_material",
                    the_item_material_db
                        .get_id_from_legacy(inv[item_index].material)
                        ->get(),
                    "name") +
                u8"製]"s;
        }
        else
        {
            s_ += u8"["s +
                cnven(i18n::s.get_m(
                    "locale.item_material",
                    the_item_material_db
                        .get_id_from_legacy(inv[item_index].material)
                        ->get(),
                    "name")) +
                u8"]"s;
        }
        if (inv[item_index].curse_state == CurseState::cursed)
        {
            s_ +=
                i18n::s.get("core.locale.item.approximate_curse_state.cursed");
        }
        if (inv[item_index].curse_state == CurseState::doomed)
        {
            s_ +=
                i18n::s.get("core.locale.item.approximate_curse_state.doomed");
        }
    }
    if (a_ == 72000)
    {
        if (inv[item_index].id == 361)
        {
            s_ += lang(u8"(移動時消滅)"s, u8"(Temporal)"s);
        }
        else if (inv[item_index].count == 0)
        {
            if (inv[item_index].param1 == 0)
            {
                s_ += lang(u8"(空っぽ)"s, u8"(Empty)"s);
            }
        }
    }
    if (a_ == 92000 && inv[item_index].param2 != 0)
    {
        s_ += lang(
            u8"(仕入れ値 "s + inv[item_index].param2 + u8"g)"s,
            u8"(Buying price: "s + inv[item_index].param2 + u8")"s);
    }
    if (inv[item_index].is_aphrodisiac())
    {
        s_ += lang(u8"(媚薬混入)"s, u8"(Aphrodisiac)"s);
    }
    if (inv[item_index].is_poisoned())
    {
        s_ += lang(u8"(毒物混入)"s, u8"(Poisoned)"s);
    }
    if (inv[item_index].has_cooldown_time() &&
        game_data.date.hours() < inv[item_index].count)
    {
        s_ += lang(
            u8"("s + (inv[item_index].count - game_data.date.hours()) +
                u8"時間)"s,
            u8"(Next: "s + (inv[item_index].count - game_data.date.hours()) +
                u8"h.)"s);
    }
    if (inv[item_index].id == 555 && inv[item_index].count != 0)
    {
        s_ += lang(u8" シリアルNo."s, u8" serial no."s) + inv[item_index].count;
    }
    if (inv[item_index].id == 544)
    {
        s_ += u8" <BGM"s + inv[item_index].param1 + u8">"s;
    }
    if (strlen_u(s_) > 66)
    {
        s_ = zentohan(s_);
    }
    skip_ = 0;
    return s_;
}



void remain_make(int ci, int cc)
{
    inv[ci].subname = cdata[cc].id;
    inv[ci].color = cdata[cc].image / 1000;
    inv[ci].weight = cdata[cc].weight;

    if (inv[ci].id == 204)
    {
        inv[ci].weight = 250 * (inv[ci].weight + 100) / 100 + 500;
        inv[ci].value = inv[ci].weight / 5;
    }
    else
    {
        inv[ci].weight = 20 * (inv[ci].weight + 500) / 500;
        inv[ci].value = cdata[cc].level * 40 + 600;
        if (the_character_db[cdata[cc].id]->rarity / 1000 < 20 &&
            cdata[cc].original_relationship < -1)
        {
            inv[ci].value = inv[ci].value *
                clamp(4 - the_character_db[cdata[cc].id]->rarity / 1000 / 5,
                      1,
                      5);
        }
    }
}


int item_stack(int inventory_id, int ci, int show_message)
{
    if (inv[ci].quality == Quality::special &&
        the_item_db[inv[ci].id]->category < 50000)
    {
        return 0;
    }

    bool did_stack = false;

    for (const auto& i : items(inventory_id))
    {
        if (i == ci || inv[i].number() == 0 || inv[i].id != inv[ci].id)
            continue;

        bool stackable;
        if (inv[i].id == 622)
            stackable = inventory_id != -1 || mode == 6 ||
                inv[i].position == inv[ci].position;
        else
            stackable =
                inv[i].almost_equals(inv[ci], inventory_id != -1 || mode == 6);

        if (stackable)
        {
            inv[i].modify_number(inv[ci].number());
            inv[ci].remove();
            ti = i;
            did_stack = true;
            break;
        }
    }

    if (did_stack)
    {
        if (mode != 6 && inv_getowner(ci) == -1)
        {
            cell_refresh(inv[ci].position.x, inv[ci].position.y);
        }
        if (show_message)
        {
            txt(i18n::s.get(
                "core.locale.item.stacked", inv[ti], inv[ti].number()));
        }
    }

    return did_stack;
}

void item_dump_desc(const Item& i)
{
    reftype = the_item_db[i.id]->category;

    dbid = i.id;
    access_item_db(2);
    access_item_db(17);

    item_load_desc(ci, p(0));

    listmax = p;
    pagemax = (listmax - 1) / pagesize;
}



void item_acid(const Character& owner, int ci)
{
    if (ci == -1)
    {
        for (const auto& body_part : owner.body_parts)
        {
            if (body_part / 10000 == 0)
            {
                break;
            }
            int i = body_part % 10000 - 1;
            if (i == -1)
            {
                continue;
            }
            if (inv[i].enhancement >= -3)
            {
                if (rnd(30) == 0)
                {
                    ci = p;
                    break;
                }
            }
        }
        if (ci == -1)
        {
            return;
        }
    }

    if (the_item_db[inv[ci].id]->category >= 50000)
    {
        return;
    }

    if (inv[ci].is_acidproof())
    {
        txt(i18n::s.get("core.locale.item.acid.immune", owner, inv[ci]));
    }
    else
    {
        txt(i18n::s.get("core.locale.item.acid.damaged", owner, inv[ci]),
            Message::color{ColorIndex::purple});
        --inv[ci].enhancement;
    }
}



bool item_fire(int owner, int ci)
{
    std::vector<int> list;

    int blanket = -1;

    if (ci != -1)
    {
        list.push_back(ci);
    }

    if (owner != -1)
    {
        if (sdata(50, owner) / 50 >= 6 ||
            cdata[owner].quality >= Quality::miracle)
        {
            return false;
        }
        for (const auto& cnt : items(owner))
        {
            if (inv[cnt].number() == 0)
            {
                continue;
            }
            if (inv[cnt].id == 567)
            {
                if (blanket == -1)
                {
                    blanket = cnt;
                }
                continue;
            }
            if (ci == -1)
            {
                list.push_back(cnt);
            }
        }
    }

    if (list.empty())
    {
        return false;
    }

    bool burned{};

    for (int cnt = 0; cnt < 3; ++cnt)
    {
        int ci_ = choice(list);
        if (inv[ci_].number() <= 0)
        {
            continue;
        }
        rowact_item(ci_);
        if (inv[ci_].is_fireproof() || inv[ci_].is_precious())
        {
            continue;
        }

        int a_ = the_item_db[inv[ci_].id]->category;
        if (a_ == 57000 && inv[ci_].param2 == 0)
        {
            if (owner == -1)
            {
                if (is_in_fov(inv[ci_].position))
                {
                    txt(i18n::s.get(
                            "core.locale.item.item_on_the_ground_get_"
                            "broiled",
                            inv[ci_]),
                        Message::color{ColorIndex::gold});
                }
            }
            else
            {
                if (is_in_fov(cdata[owner]))
                {
                    txt(i18n::s.get(
                            "core.locale.item.item_on_the_ground_get_"
                            "broiled",
                            inv[ci_],
                            cdata[owner]),
                        Message::color{ColorIndex::gold});
                }
            }
            make_dish(ci_, rnd(5) + 1);
            burned = true;
            continue;
        }

        if (a_ == 72000 || a_ == 59000 || a_ == 68000 ||
            inv[ci_].quality >= Quality::miracle)
        {
            continue;
        }

        if (inv[ci_].body_part != 0)
        {
            if (rnd(2))
            {
                continue;
            }
        }

        if (a_ != 56000 && a_ != 80000 && a_ != 55000 && a_ != 53000 &&
            a_ != 54000)
        {
            if (rnd(4))
            {
                continue;
            }
            if (owner != -1)
            {
                if (rnd(4))
                {
                    continue;
                }
            }
        }

        if (blanket != -1)
        {
            item_separate(blanket);
            if (is_in_fov(cdata[owner]))
            {
                txt(i18n::s.get(
                    "core.locale.item.fireproof_blanket_protects_item",
                    inv[blanket],
                    cdata[owner]));
            }
            if (inv[blanket].count > 0)
            {
                --inv[blanket].count;
            }
            else if (rnd(20) == 0)
            {
                inv[blanket].modify_number(-1);
                if (is_in_fov(cdata[owner]))
                {
                    txt(i18n::s.get(
                        "core.locale.item.fireproof_blanket_turns_to_dust",
                        inv[blanket]));
                }
                break;
            }
            continue;
        }

        int p_ = rnd(inv[ci_].number()) / 2 + 1;
        if (owner != -1)
        {
            if (inv[ci_].body_part != 0)
            {
                if (is_in_fov(cdata[owner]))
                {
                    txt(i18n::s.get(
                            "core.locale.item.item_someone_equips_turns_to_"
                            "dust",
                            itemname(inv[ci_].index, p_),
                            p_,
                            cdata[owner]),
                        Message::color{ColorIndex::purple});
                }
                cdata[owner].body_parts[inv[ci_].body_part - 100] =
                    cdata[owner].body_parts[inv[ci_].body_part - 100] / 10000 *
                    10000;
                inv[ci_].body_part = 0;
                chara_refresh(owner);
            }
            else if (is_in_fov(cdata[owner]))
            {
                txt(i18n::s.get(
                        "core.locale.item.someones_item_turns_to_dust",
                        itemname(inv[ci_].index, p_, 1),
                        p_,
                        cdata[owner]),
                    Message::color{ColorIndex::purple});
            }
        }
        else if (is_in_fov(inv[ci_].position))
        {
            txt(i18n::s.get(
                    "core.locale.item.item_on_the_ground_turns_to_dust",
                    itemname(inv[ci_].index, p_),
                    p_),
                Message::color{ColorIndex::purple});
        }
        inv[ci_].modify_number(-p_);
        cell_refresh(inv[ci_].position.x, inv[ci_].position.y);
        burned = true;
    }

    refresh_burden_state();
    return burned;
}



void mapitem_fire(int x, int y)
{
    if (cell_data.at(x, y).item_appearances_actual == 0)
    {
        return;
    }

    int ci = -1;
    for (const auto& cnt : items(-1))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].position == Position{x, y})
        {
            ci = cnt;
            break;
        }
    }
    if (ci != -1)
    {
        const auto burned = item_fire(-1, ci);
        if (burned)
        {
            if (cell_data.at(x, y).mef_index_plus_one == 0)
            {
                mef_add(x, y, 5, 24, rnd(10) + 5, 100, cc);
            }
        }
        cell_refresh(x, y);
    }
}



bool item_cold(int owner, int ci)
{
    int blanket = -1;
    std::vector<int> list;
    if (ci != -1)
    {
        list.push_back(ci);
    }
    if (owner != -1)
    {
        if (sdata(51, owner) / 50 >= 6 ||
            cdata[owner].quality >= Quality::miracle)
        {
            return false;
        }
        for (const auto& cnt : items(owner))
        {
            if (inv[cnt].number() == 0)
            {
                continue;
            }
            if (inv[cnt].id == 568)
            {
                if (blanket == -1)
                {
                    blanket = cnt;
                }
                continue;
            }
            if (ci == -1)
            {
                list.push_back(cnt);
            }
        }
    }
    if (list.empty())
    {
        return false;
    }

    bool broken{};
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        int ci_ = choice(list);
        if (inv[ci_].number() <= 0)
        {
            continue;
        }
        rowact_item(ci_);
        if (inv[ci_].is_precious())
        {
            continue;
        }

        int a_ = the_item_db[inv[ci_].id]->category;
        if (a_ == 72000 || a_ == 59000 || a_ == 68000)
        {
            continue;
        }
        if (inv[ci_].quality >= Quality::miracle || inv[ci_].body_part != 0)
        {
            continue;
        }
        if (a_ != 52000)
        {
            if (rnd(30))
            {
                continue;
            }
        }
        if (blanket != -1)
        {
            item_separate(blanket);
            if (is_in_fov(cdata[owner]))
            {
                txt(i18n::s.get(
                    "core.locale.item.coldproof_blanket_protects_item",
                    inv[blanket],
                    cdata[owner]));
            }
            if (inv[blanket].count > 0)
            {
                --inv[blanket].count;
            }
            else if (rnd(20) == 0)
            {
                inv[blanket].modify_number(-1);
                if (is_in_fov(cdata[owner]))
                {
                    txt(i18n::s.get(
                        "core.locale.item.coldproof_blanket_is_broken_to_"
                        "pieces",
                        inv[blanket]));
                }
                break;
            }
            continue;
        }
        int p_ = rnd(inv[ci_].number()) / 2 + 1;
        if (owner != -1)
        {
            if (is_in_fov(cdata[owner]))
            {
                txt(i18n::s.get(
                        "core.locale.item.someones_item_breaks_to_pieces",
                        itemname(inv[ci_].index, p_, 1),
                        p_,
                        cdata[owner]),
                    Message::color{ColorIndex::purple});
            }
        }
        else if (is_in_fov(inv[ci_].position))
        {
            txt(i18n::s.get(
                    "core.locale.item.item_on_the_ground_breaks_to_pieces",
                    itemname(inv[ci_].index, p_),
                    p_),
                Message::color{ColorIndex::purple});
        }
        inv[ci_].modify_number(-p_);
        broken = true;
    }

    refresh_burden_state();
    return broken;
}



void mapitem_cold(int x, int y)
{
    if (cell_data.at(x, y).item_appearances_actual == 0)
    {
        return;
    }
    int ci = -1;
    for (const auto& cnt : items(-1))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].position == Position{x, y})
        {
            ci = cnt;
            break;
        }
    }
    if (ci != -1)
    {
        item_cold(-1, ci);
        cell_refresh(x, y);
    }
}



int get_random_inv(int owner)
{
    const auto tmp = inv_getheader(owner);
    return tmp.first + rnd(tmp.second);
}

std::pair<int, int> inv_getheader(int owner)
{
    if (owner == 0)
    {
        return {0, 200};
    }
    else if (owner == -1)
    {
        return {ELONA_ITEM_ON_GROUND_INDEX, 400};
    }
    else
    {
        return {200 + 20 * (owner - 1), 20};
    }
}



int inv_getowner(int inv_id)
{
    if (inv_id < 200)
    {
        return 0;
    }
    if (inv_id >= ELONA_ITEM_ON_GROUND_INDEX)
    {
        return -1;
    }
    return (inv_id - 200) / 20 + 1;
}



int inv_find(int id, int owner)
{
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() == 0)
        {
            continue;
        }
        if (inv[cnt].id == id)
        {
            return cnt;
        }
    }
    return -1; // Not found
}

bool inv_getspace(int owner)
{
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() == 0)
        {
            return true;
        }
    }
    return false;
}

int inv_sum(int owner)
{
    int n{};
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() != 0)
        {
            ++n;
        }
    }
    return n;
}

int inv_compress(int owner)
{
    int number_of_deleted_items{};
    for (int i = 0; i < 100; ++i)
    {
        int threshold = 200 * (i * i + 1);
        for (const auto& cnt : items(owner))
        {
            if (inv[cnt].number() != 0)
            {
                if (!inv[cnt].is_precious() && inv[cnt].value < threshold)
                {
                    inv[cnt].remove();
                    ++number_of_deleted_items;
                    if (inv[cnt].position.x >= 0 &&
                        inv[cnt].position.x < map_data.width &&
                        inv[cnt].position.y >= 0 &&
                        inv[cnt].position.y < map_data.height)
                    {
                        cell_refresh(inv[cnt].position.x, inv[cnt].position.y);
                    }
                }
            }
            if (number_of_deleted_items > 10)
            {
                break;
            }
        }
        if (number_of_deleted_items > 10)
        {
            break;
        }
    }

    int slot = -1;
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() == 0)
        {
            slot = cnt;
            break;
        }
    }

    if (slot == -1)
    {
        // Destroy 1 existing item forcely.
        slot = get_random_inv(owner);
        inv[slot].remove();
        if (mode != 6)
        {
            if (inv[slot].position.x >= 0 &&
                inv[slot].position.x < map_data.width &&
                inv[slot].position.y >= 0 &&
                inv[slot].position.y < map_data.height)
            {
                cell_refresh(inv[slot].position.x, inv[slot].position.y);
            }
        }
    }

    return slot;
}

int inv_getfreeid(int owner)
{
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() == 0)
        {
            return cnt;
        }
    }
    if (owner == -1 && mode != 6)
    {
        txt(i18n::s.get("core.locale.item.items_are_destroyed"));
        return inv_compress(owner);
    }
    return -1;
}

int inv_weight(int owner)
{
    int weight{};
    if (owner == 0)
    {
        game_data.cargo_weight = 0;
    }
    for (const auto& cnt : items(owner))
    {
        if (inv[cnt].number() != 0)
        {
            if (inv[cnt].weight >= 0)
            {
                weight += inv[cnt].weight * inv[cnt].number();
            }
            else if (owner == 0)
            {
                game_data.cargo_weight += -inv[cnt].weight * inv[cnt].number();
            }
        }
    }
    return weight;
}

int inv_getfreeid_force()
{
    p = inv_getfreeid(tc);
    if (p != -1)
    {
        return p;
    }
    for (int cnt = 0; cnt < 100; ++cnt)
    {
        p = rnd(invrange) + invhead;
        if (inv[p].body_part == 0)
        {
            inv[p].remove();
            if (cdata[tc].item_which_will_be_used == p)
            {
                cdata[tc].item_which_will_be_used = 0;
            }
            break;
        }
    }
    return p;
}



void item_drop(Item& item_in_inventory, int num, bool building_shelter)
{
    ti = inv_getfreeid(-1);
    if (ti == -1)
    {
        txt(i18n::s.get("core.locale.action.drop.too_many_items"));
        update_screen();
        return;
    }

    item_copy(item_in_inventory.index, ti);
    inv[ti].position = cdata[cc].position;
    inv[ti].set_number(num);
    itemturn(ti);

    if (building_shelter)
    {
        inv[ti].own_state = 3;
        inv[ti].count = game_data.next_shelter_serial_id + 100;
        ++game_data.next_shelter_serial_id;
    }
    else
    {
        snd("core.drop1");
        txt(i18n::s.get("core.locale.action.drop.execute", itemname(ti, num)));
    }

    if (inv[ti].id == 516) // Water
    {
        int altar = item_find(60002, 0);
        if (altar != -1)
        {
            // The altar is your god's.
            if (core_god::int2godid(inv[altar].param1) == cdata[cc].god_id)
            {
                if (inv[ti].curse_state != CurseState::blessed)
                {
                    snd("core.pray1");
                    inv[ti].curse_state = CurseState::blessed;
                    txt(i18n::s.get("core.locale.action.drop.water_is_blessed"),
                        Message::color{ColorIndex::green});
                }
            }
        }
    }

    item_stack(-1, ti);
    item_in_inventory.modify_number(-num);

    refresh_burden_state();
    cell_refresh(inv[ti].position.x, inv[ti].position.y);
    screenupdate = -1;
    update_screen();

    if (area_data[game_data.current_map].id == mdata_t::MapId::museum)
    {
        if (mode == 0)
        {
            update_museum();
        }
    }
    if (game_data.current_map == mdata_t::MapId::your_home)
    {
        if (mode == 0)
        {
            calc_home_rank();
        }
    }
    if (inv[ti].id == 255)
    {
        map_data.play_campfire_sound = 1;
        play_music();
    }
}



void item_build_shelter(Item& shelter)
{
    item_drop(shelter, 1, true);
}



} // namespace elona
