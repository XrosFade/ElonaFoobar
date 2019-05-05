#include "lua_class.hpp"

#include "lua_class_ability.hpp"
#include "lua_class_area.hpp"
#include "lua_class_character.hpp"
#include "lua_class_date_time.hpp"
#include "lua_class_game_data.hpp"
#include "lua_class_item.hpp"
#include "lua_class_map_data.hpp"
#include "lua_class_map_generator.hpp"
#include "lua_class_position.hpp"

namespace elona
{
namespace lua
{

void LuaApiClasses::bind(sol::state& lua)
{
    LuaAbility::bind(lua);
    LuaArea::bind(lua);
    LuaCharacter::bind(lua);
    LuaDateTime::bind(lua);
    LuaGameData::bind(lua);
    LuaItem::bind(lua);
    LuaMapData::bind(lua);
    LuaMapGenerator::bind(lua);
    LuaPosition::bind(lua);
}

void LuaApiClasses::bind_api(sol::state& lua, sol::table& core)
{
    core["LuaPosition"] = lua["LuaPosition"];
}

} // namespace lua
} // namespace elona
