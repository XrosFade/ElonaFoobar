#include "ability.hpp"
#include "calc.hpp"
#include "character.hpp"
#include "class.hpp"
#include "draw.hpp"
#include "elona.hpp"
#include "food.hpp"
#include "i18n.hpp"
#include "lua_env/interface.hpp"
#include "lua_env/lua_env.hpp"
#include "message.hpp"
#include "race.hpp"
#include "random.hpp"
#include "text.hpp"
#include "variables.hpp"



namespace elona
{

int access_character_info()
{
    const auto data = the_character_db[dbid];
    if (!data)
        return 0;

    switch (dbmode)
    {
    case 16:
        switch (dbspec)
        {
        case 2:
            refstr = i18n::s.get_m(
                "locale.chara",
                the_character_db.get_id_from_legacy(dbid)->get(),
                "name");
            return 0;
        case 3: return data->item_type;
        case 8: refstr = data->filter; return 0;
        default: assert(0);
        }
    case 3:
        cdata[rc].id = dbid;
        cdata[rc].level = initlv != 0 ? initlv : data->level;
        if (voidlv != 0)
        {
            cdata[rc].level = voidlv * (100 + data->level * 2) / 100;
        }
        cdata[rc].ai_calm = data->ai_calm;
        cdata[rc].ai_heal = data->ai_heal;
        cdata[rc].ai_move = data->ai_move;
        cdata[rc].ai_dist = data->ai_dist;
        cdata[rc].ai_act_sub_freq = data->ai_act_sub_freq;
        cdata[rc].normal_actions = data->normal_actions;
        cdata[rc].special_actions = data->special_actions;
        creaturepack = data->creaturepack;
        cdata[rc].can_talk = data->can_talk;
        cdatan(0, rc) = i18n::s.get_m(
            "locale.chara",
            the_character_db.get_id_from_legacy(dbid)->get(),
            "name");
        if (data->has_random_name)
        {
            cdatan(0, rc) = i18n::s.get(
                "core.locale.chara.job.own_name", cdatan(0, rc), random_name());
            cdata[rc].has_own_name() = true;
        }
        cdata[rc].original_relationship = cdata[rc].relationship =
            data->original_relationship;
        if (!data->race.empty())
        {
            access_race_info(3, data->race);
        }
        if (!data->class_.empty())
        {
            access_class_info(3, data->class_);
        }
        cdata[rc].element_of_unarmed_attack = data->element_of_unarmed_attack;

        for (const auto& pair : data->resistances)
        {
            if (const auto ability_data = the_ability_db[pair.first])
            {
                sdata(ability_data->legacy_id, rc) = pair.second;
            }
            else
            {
                // Skip the resistance if undefined.
                ELONA_WARN("lua.data")
                    << "Undefined resistance ID: " << pair.first
                    << " (character " << data->id << ")";
            }
        }

        if (data->sex != -1)
        {
            cdata[rc].sex = data->sex;
        }
        if (data->image != 0)
        {
            cdata[rc].image = data->image;
        }
        if (cdata[rc].sex == 0 && data->male_image != 0)
        {
            cdata[rc].image = data->male_image;
        }
        if (cdata[rc].sex == 1 && data->female_image != 0)
        {
            cdata[rc].image = data->female_image;
        }
        if (cdata[rc].sex == 0)
        {
            cdata[rc].portrait = data->portrait_male;
        }
        else
        {
            cdata[rc].portrait = data->portrait_female;
        }
        {
            int color = generate_color(data->color, cdata[rc].id);
            cdata[rc].image += color * 1000;
        }
        eqammo(0) = data->eqammo_0;
        eqammo(1) = data->eqammo_1;
        eqmultiweapon = data->eqmultiweapon;
        eqrange(0) = data->eqrange_0;
        eqrange(1) = data->eqrange_1;
        eqring1 = data->eqring1;
        eqtwohand = data->eqtwohand;
        eqweapon1 = data->eqweapon1;
        if (data->fixlv == 6)
        {
            fixlv = Quality::special;
        }
        cspecialeq = data->cspecialeq;
        cdata[rc].damage_reaction_info = data->damage_reaction_info;
        return 0;
    default: break;
    }


    switch (dbmode)
    {
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
        if (dbid == 176)
        {
            std::string text =
                i18n::s.get_enum("core.locale.ui.onii", cdata.player().sex);

            if (dbmode == 100)
            {
                if (jp)
                {
                    txt(u8"「" + text + u8"ちゃんー」",
                        u8"「" + text + u8"ちゃん！」",
                        u8"「" + text + u8"ちゃ〜ん」",
                        u8"「" + text + u8"ちゃんっ」",
                        u8"「" + text + u8"ちゃん？」",
                        u8"「" + text + u8"〜ちゃん」",
                        u8"「" + text + u8"ちゃん♪」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
            if (dbmode == 104)
            {
                if (jp)
                {
                    txt(u8"「おかえり、" + text + u8"ちゃん！」",
                        u8"「おかえりなさーい、" + text + u8"ちゃん♪」",
                        u8"「待ってたよ、" + text + u8"ちゃん」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
        }
        else if (dbid == 249)
        {
            std::string text =
                i18n::s.get_enum("core.locale.ui.onii", cdata.player().sex);

            if (dbmode == 100)
            {
                if (jp)
                {
                    txt(u8"「" + text + u8"ちゃんー」",
                        u8"「" + text + u8"ちゃん！」",
                        u8"「" + text + u8"ちゃ〜ん」",
                        u8"「" + text + u8"ちゃんっ」",
                        u8"「" + text + u8"ちゃん？」",
                        u8"「" + text + u8"〜ちゃん」",
                        u8"「" + text + u8"ちゃん♪」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
            if (dbmode == 104)
            {
                if (jp)
                {
                    txt(u8"「おかえり、" + text + u8"ちゃん！」",
                        u8"「おかえりなさーい、" + text + u8"ちゃん♪」",
                        u8"「待ってたよ、" + text + u8"ちゃん」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
        }
        else if (dbid == 210)
        {
            std::string text =
                i18n::s.get_enum("core.locale.ui.onii", cdata.player().sex);

            if (dbmode == 100)
            {
                if (jp)
                {
                    txt(u8"「" + text + u8"ちゃんー」",
                        u8"「" + text + u8"ちゃん！」",
                        u8"「" + text + u8"ちゃ〜ん」",
                        u8"「" + text + u8"ちゃんっ」",
                        u8"「" + text + u8"ちゃん？」",
                        u8"「" + text + u8"〜ちゃん」",
                        u8"「" + text + u8"ちゃん♪」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
            if (dbmode == 104)
            {
                if (jp)
                {
                    txt(u8"「おかえりにゃ、" + text + u8"ちゃん！」",
                        u8"「おかえりなさいにゃー、" + text + u8"ちゃん♪」",
                        u8"「待ってたにゃ、" + text + u8"ちゃん」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
        }
        else if (dbid == 205)
        {
            std::string text =
                i18n::s.get_enum("core.locale.ui.syujin", cdata.player().sex);

            if (dbmode == 100)
            {
                if (jp)
                {
                    txt(u8"「" + text + u8"〜」",
                        text + u8"〜",
                        u8"「用事はありませんか♪」",
                        u8"メイドの熱い視線を感じる…",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
            if (dbmode == 102)
            {
                if (jp)
                {
                    txt(u8"「ダメぇ！」",
                        u8"「" + text + u8"ー！」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
            if (dbmode == 104)
            {
                if (jp)
                {
                    txt(u8"「おかえりなさいませ、" + text + u8"〜」",
                        u8"「おかえりなさいまし〜」",
                        Message::color{ColorIndex::cyan});
                    return 1;
                }
            }
        }
        {
            const auto chara_id =
                the_character_db.get_id_from_legacy(dbid)->get();
            const auto dialog_id = "text_" + std::to_string(dbmode);
            if (const auto text =
                    i18n::s.get_m_optional("locale.chara", chara_id, dialog_id))
            {
                txt(*text, Message::color{ColorIndex::cyan});
            }
        }
        break;
    default: break;
    }

    if (dbmode == 12 && data->corpse_eating_callback)
    {
        lua::call(*data->corpse_eating_callback, lua::handle(cdata[cc]));
        return -1;
    }

    switch (dbid)
    {
    case 2:
        if (dbmode == 4)
        {
            eqweapon1 = 63;
            return 0;
        }
        return 0;
    case 23:
        if (dbmode == 4)
        {
            eqweapon1 = 64;
            return 0;
        }
        return 0;
    case 28:
        if (dbmode == 4)
        {
            eqweapon1 = 73;
            return 0;
        }
        return 0;
    case 351:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 232;
            return 0;
        }
        return 0;
    case 33:
        if (dbmode == 4)
        {
            eqweapon1 = 206;
            return 0;
        }
        return 0;
    case 34:
        if (dbmode == 4)
        {
            eqweapon1 = 1;
            eqrange = 207;
            eqammo(0) = 25001;
            eqammo(1) = 3;
            return 0;
        }
        return 0;
    case 141:
        if (dbmode == 4)
        {
            eqweapon1 = 358;
            return 0;
        }
        return 0;
    case 143:
        if (dbmode == 4)
        {
            eqweapon1 = 359;
            return 0;
        }
        return 0;
    case 144:
        if (dbmode == 4)
        {
            eqweapon1 = 356;
            return 0;
        }
        return 0;
    case 145:
        if (dbmode == 4)
        {
            eqring1 = 357;
            return 0;
        }
        return 0;
    case 336:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 739;
            return 0;
        }
        return 0;
    case 338:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 739;
            return 0;
        }
        return 0;
    case 339:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 739;
            return 0;
        }
        return 0;
    case 342:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 739;
            return 0;
        }
        return 0;
    case 340:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            eqweapon1 = 739;
            return 0;
        }
        return 0;
    case 299:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 300:
        if (dbmode == 4)
        {
            eqweapon1 = 695;
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 309:
        if (dbmode == 4)
        {
            eqmultiweapon = 2;
            return 0;
        }
        return 0;
    case 310:
        if (dbmode == 4)
        {
            eqmultiweapon = 266;
            return 0;
        }
        return 0;
    case 311:
        if (dbmode == 4)
        {
            eqmultiweapon = 224;
            return 0;
        }
        return 0;
    case 307:
        if (dbmode == 4)
        {
            eqweapon1 = 735;
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 308:
        if (dbmode == 4)
        {
            eqweapon1 = 735;
            return 0;
        }
        return 0;
    case 50:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 90:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 151:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 156:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 303:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 163:
        if (dbmode == 4)
        {
            eqrange = 210;
            return 0;
        }
        return 0;
    case 170:
        if (dbmode == 4)
        {
            eqrange = 210;
            return 0;
        }
        return 0;
    case 177:
        if (dbmode == 4)
        {
            eqweapon1 = 1;
            eqrange = 514;
            eqammo(0) = 25030;
            eqammo(1) = 3;
            return 0;
        }
        return 0;
    case 212:
        if (dbmode == 4)
        {
            eqweapon1 = 56;
            return 0;
        }
        return 0;
    case 301:
        if (dbmode == 4)
        {
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 317:
        if (dbmode == 4)
        {
            eqweapon1 = 232;
            eqtwohand = 1;
            return 0;
        }
        return 0;
    case 318:
        if (dbmode == 4)
        {
            eqrange(0) = 496;
            eqrange(1) = 4;
            eqammo(0) = 25020;
            eqammo(1) = 3;
            return 0;
        }
        return 0;
    default: return 0;
    }
}

} // namespace elona
