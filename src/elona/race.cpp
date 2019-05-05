#include "race.hpp"
#include "ability.hpp"
#include "character.hpp"
#include "elona.hpp"
#include "i18n.hpp"
#include "random.hpp"
#include "variables.hpp"



namespace elona
{

std::vector<std::reference_wrapper<const RaceData>> race_get_available(
    bool is_extra_race)
{
    std::vector<std::reference_wrapper<const RaceData>> ret;
    for (const auto& race : the_race_db.values())
    {
        if (race.is_extra == is_extra_race)
        {
            ret.emplace_back(race);
        }
    }
    range::sort(ret, [](const auto& a, const auto& b) {
        return a.get().ordering < b.get().ordering;
    });
    return ret;
}



int access_race_info(int dbmode, const std::string& race_id)
{
    auto data = the_race_db[race_id];
    if (!data)
        return 0;

    switch (dbmode)
    {
    case 3: break;
    case 11:
    {
        buff = i18n::s.get_m_optional("locale.race", race_id, "description")
                   .get_value_or("");
        ref1 = data->male_image;
        ref2 = data->female_image;
        return 0;
    }
    default: assert(0);
    }

    cdatan(2, rc) = race_id;
    cdata[rc].melee_attack_type = data->melee_attack_type;
    cdata[rc].special_attack_type = data->special_attack_type;
    cdata[rc].dv_correction_value = data->dv_multiplier;
    cdata[rc].pv_correction_value = data->pv_multiplier;

    cdata[rc].birth_year = game_data.date.year -
        (rnd(data->max_age - data->min_age + 1) + data->min_age);
    cdata[rc].height = data->height;
    if (mode == 1)
    {
        cdata[rc].sex = cmsex;
    }
    else
    {
        if (rnd(100) < data->male_ratio)
        {
            cdata[rc].sex = 0;
        }
        else
        {
            cdata[rc].sex = 1;
        }
    }

    if (cdata[rc].sex == 0)
    {
        cdata[rc].image = data->male_image;
    }
    else
    {
        cdata[rc].image = data->female_image;
    }

    if (data->is_made_of_rock)
    {
        cdata[rc].breaks_into_debris() = true;
    }

    {
        size_t i{};
        for (const auto& p : data->body_parts)
        {
            cdata[rc].body_parts[i] = p * 10'000;
            ++i;
        }
        cdata[rc].body_parts[i] = 10 * 10'000;
        ++i;
        cdata[rc].body_parts[i] = 11 * 10'000;
        ++i;
    }

    for (const auto& pair : data->skills)
    {
        if (const auto ability_data = the_ability_db[pair.first])
        {
            chara_init_skill(cdata[rc], ability_data->legacy_id, pair.second);
        }
        else
        {
            // Skip the skill if undefined.
            ELONA_WARN("lua.data") << "Undefined skill ID: " << pair.first
                                   << " (race " << race_id << ")";
        }
    }

    for (const auto& pair : data->resistances)
    {
        if (const auto ability_data = the_ability_db[pair.first])
        {
            sdata(ability_data->legacy_id, rc) = pair.second;
        }
        else
        {
            // Skip the resistance if undefined.
            ELONA_WARN("lua.data") << "Undefined resistance ID: " << pair.first
                                   << " (race " << race_id << ")";
        }
    }

    return 0;
}

} // namespace elona
