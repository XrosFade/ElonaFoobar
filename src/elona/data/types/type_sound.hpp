#pragma once
#include "../lua_lazy_cache.hpp"

namespace elona
{

struct SoundData
{
    SharedId id;
    int legacy_id;
    fs::path file;
};



ELONA_DEFINE_LUA_DB(SoundDB, SoundData, true, "core.sound")


extern SoundDB the_sound_db;

} // namespace elona
