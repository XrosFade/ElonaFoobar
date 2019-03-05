#include "profile_manager.hpp"
#include "../filesystem.hpp"
#include "../log.hpp"



namespace elona
{
namespace profile
{

void ProfileManager::init(const ProfileId& profile_id)
{
    ELONA_LOG("profile") << "Initialize with '" << profile_id << "'.";

    if (!exists(profile_id))
    {
        ELONA_LOG("profile") << "Not found '" << profile_id << "'.";
        create(profile_id);
    }

    load(profile_id);
}



void ProfileManager::load(const ProfileId& profile_id)
{
    ELONA_LOG("profile") << "Load '" << profile_id << "'.";

    filesystem::dir::set_profile_directory(get_dir_for(profile_id));
}



ProfileManager& ProfileManager::instance()
{
    static ProfileManager the_instance;
    return the_instance;
}



void ProfileManager::create(
    const ProfileId& new_profile_id,
    const ProfileId& base_profile_id)
{
    ELONA_LOG("profile") << "Create '" << new_profile_id << "' "
                         << " from '" << base_profile_id << "'.";

    const auto from = get_dir_for(base_profile_id);
    const auto to = get_dir_for(new_profile_id);
    filesystem::copy_recursively(from, to);
}



ProfileId ProfileManager::generate_new_id()
{
    for (int i = 1;; ++i)
    {
        const auto id = "profile_" + std::to_string(i);
        const auto dir = get_dir_for(id);
        if (!fs::exists(dir))
        {
            return id;
        }
    }
}



void ProfileManager::delete_(const ProfileId& profile_id)
{
    const auto dir = get_dir_for(profile_id);
    fs::remove_all(dir);
}

} // namespace profile
} // namespace elona
