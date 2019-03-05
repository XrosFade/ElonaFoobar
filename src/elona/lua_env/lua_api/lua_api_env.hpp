#pragma once

#include <string>
#include "lua_api_common.hpp"



namespace elona
{
namespace lua
{

/**
 * @luadoc
 *
 * Information about Elona foobar and mod engine environment.
 */
namespace LuaApiEnv
{

void bind(sol::table&);

} // namespace LuaApiEnv
} // namespace lua
} // namespace elona
