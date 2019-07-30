#include "testing.hpp"

#include <sstream>

#include "../version.hpp"
#include "config/config.hpp"
#include "ctrl_file.hpp"
#include "data/types/type_item.hpp"
#include "data/types/type_music.hpp"
#include "data/types/type_sound.hpp"
#include "draw.hpp"
#include "gdata.hpp"
#include "i18n.hpp"
#include "init.hpp"
#include "log.hpp"
#include "lua_env/event_manager.hpp"
#include "lua_env/lua_env.hpp"
#include "lua_env/lua_event/base_event.hpp"
#include "profile/profile_manager.hpp"
#include "save.hpp"
#include "variables.hpp"



namespace elona
{
namespace testing
{

const std::string save_dir = "tests/data/save";
const std::string player_id = "sav_testbed";

void reset_state();

fs::path get_test_data_path()
{
    return filesystem::dir::exe() / "tests" / "data";
}

fs::path get_mods_path()
{
    return get_test_data_path() / "mods";
}

void load_previous_savefile()
{
    testing::reset_state();
    // This file was saved directly after the dialog at the start of the game.
    elona::playerid = "sav_foobar_test";
    filesystem::dir::set_base_save_directory(filesystem::path(save_dir));
    load_save_data();
    elona::firstturn = 1;
    elona::mode =
        3; // begin the game as if it were loaded from a save; load inv_xxx.s2
    initialize_map();
}

void save_reset_and_reload()
{
    testing::save();
    testing::reset_state();
    testing::load();
}

void save_and_reload()
{
    testing::save();
    testing::load();
}

void save()
{
    filesystem::dir::set_base_save_directory(filesystem::path(save_dir));
    save_game();
}

void load()
{
    elona::firstturn = 1;
    load_save_data();
    elona::mode = 3;
    initialize_map();
}

void load_translations(const std::string& hcl)
{
    i18n::s.clear();

    std::stringstream ss(hcl);
    i18n::s.load(ss, "test.hcl", "test");
}

// Similar to load_translations, but does not reset all i18n resources.
void reinit_core_and_load_translations(const std::string& hcl)
{
    initialize_i18n();

    std::stringstream ss(hcl);
    i18n::s.load(ss, "test.hcl", "test");
}

void configure_lua()
{
    lua::lua.reset(new lua::LuaEnv());
    initialize_lua();

    sol::table Testing = lua::lua->get_state()->create_named_table("Testing");
    Testing.set_function("start_in_debug_map", start_in_debug_map);
    Testing.set_function("reset_state", reset_state);
    Testing.set_function("load_translations", load_translations);
    Testing.set_function(
        "reinit_core_and_load_translations", reinit_core_and_load_translations);
}

void start_in_map(int map, int level)
{
    filesystem::dir::set_base_save_directory(filesystem::path(save_dir));

    lua::lua->clear();
    initialize_debug_globals();

    elona::playerid = player_id;
    fs::remove_all(filesystem::dir::save(player_id));
    fs::remove_all(filesystem::dir::tmp());
    fs::create_directory(filesystem::dir::tmp());
    writeloadedbuff_clear();
    Save::instance().clear();

    game_data.current_map = map;
    game_data.current_dungeon_level = level;
    init_fovlist();
    elona::mode = 2;
    initialize_map();

    save_game();
}

void start_in_debug_map()
{
    start_in_map(499, 2);
}

void run_in_temporary_map(int map, int dungeon_level, std::function<void()> f)
{
    auto previous_map = game_data.current_map;
    auto previous_dungeon_level = game_data.current_dungeon_level;
    auto previous_x = cdata.player().position.x;
    auto previous_y = cdata.player().position.y;
    game_data.destination_map = map;
    game_data.destination_dungeon_level = dungeon_level;
    elona::levelexitby = 2;
    exit_map();
    initialize_map();

    f();

    elona::mapstartx = previous_x;
    elona::mapstarty = previous_y;
    game_data.destination_map = previous_map;
    game_data.destination_dungeon_level = previous_dungeon_level;
    elona::levelexitby = 2;
    exit_map();
    initialize_map();
}

void pre_init()
{
    log::Logger::instance().init();
    profile::ProfileManager::instance().init(u8"testing");

    const fs::path source_config_file = get_test_data_path() / "config.hcl";
    const fs::path config_file =
        filesystem::dir::current_profile() / "config.hcl";
    fs::copy_file(
        source_config_file, config_file, fs::copy_option::overwrite_if_exists);

    initialize_config_defs();
    initialize_config_preload();

    title(u8"Elona Foobar version "s + latest_version.short_string());

    init_assets();
    filesystem::dir::set_base_save_directory(fs::path("save"));
    initialize_config();

    configure_lua();
    initialize_i18n();

    initialize_elona();

    Config::instance().is_test = true;

    lua::lua->get_event_manager().trigger(
        lua::BaseEvent("core.game_initialized"));
}

void post_run()
{
    filesystem::dir::set_base_save_directory(filesystem::path(save_dir));
    fs::remove_all(filesystem::dir::save(player_id));
    fs::remove_all(filesystem::dir::tmp());
    writeloadedbuff_clear();
    Save::instance().clear();
    fs::create_directory(filesystem::dir::tmp());
    finish_elona();
}

void reset_state()
{
    lua::lua->clear();
    configure_lua();
    initialize_i18n();
    initialize_elona();

    // reset translations
    elona::jp = 1;
    elona::en = 0;
    set_item_info();

    Config::instance().is_test = true;

    lua::lua->get_event_manager().trigger(
        lua::BaseEvent("core.game_initialized"));
}

} // namespace testing
} // namespace elona
