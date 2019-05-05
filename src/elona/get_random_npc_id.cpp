#include "character.hpp"
#include "elona.hpp"
#include "random.hpp"
#include "variables.hpp"


namespace
{



struct CharacterId
{
    int id;
    int cnpcid; // It is used only if id equals 343.
};


} // namespace



namespace elona
{


void get_random_npc_id()
{
    WeightedRandomSampler<CharacterId> sampler;

    for (const auto& data : the_character_db.values())
    {
        if (data.level > objlv)
            continue;
        if (fltselect != data.fltselect)
            continue;
        if (fltselect == 2 && npcmemory(1, data.legacy_id) != 0)
            continue;
        if (flttypemajor != 0 && flttypemajor != data.category)
            continue;
        if (!fltnrace(0).empty() && fltnrace(0) != data.race)
            continue;
        if (filtermax != 0)
        {
            // FIXME: use std::all_of
            bool ok = true;
            for (int i = 0; i < filtermax; ++i)
            {
                if (data.filter.find(filtern(i)) == std::string::npos)
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;
        }
        sampler.add(
            {data.legacy_id, 0},
            data.rarity /
                    (500 + std::abs(data.level - objlv) * data.coefficient) +
                1);
    }

    if (!cmshade)
    {
        for (int i = 0; i < usernpcmax; ++i)
        {
            if (userdata(2, i) > objlv)
                continue;
            if (fltselect != userdata(12, i))
                continue;
            if (flttypemajor != 0)
                continue;
            if (!fltnrace(0).empty() && fltnrace(0) != userdatan(2, i))
                continue;
            if (filtermax != 0)
            {
                // FIXME: use std::all_of
                bool ok = false;
                for (int j = 0; j < filtermax; ++j)
                {
                    if (userdatan(4, i).find(filtern(j)) == std::string::npos)
                    {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;
            }
            sampler.add(
                {343, i},
                clamp(userdata(6, i), 1'000, 500'000) /
                        (500 + std::abs(userdata(2, i) - objlv) * 400) +
                    1);
        }
    }

    const auto id_info = sampler.get().value_or(CharacterId{0, 0});
    dbid = id_info.id;
    if (id_info.id == 343)
    {
        cdata[rc].cnpc_id = id_info.cnpcid;
    }
}



} // namespace elona
