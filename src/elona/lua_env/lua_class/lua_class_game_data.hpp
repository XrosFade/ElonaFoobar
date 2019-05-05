#pragma once
#include "../lua_api/lua_api_common.hpp"

namespace elona
{
namespace lua
{

/**
 * @luadoc
 *
 * Contains parameters for the current game save.
 */
namespace LuaGameData
{
void bind(sol::state&);
}

} // namespace lua
} // namespace elona
