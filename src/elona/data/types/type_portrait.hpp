#pragma once

#include "../../../util/natural_order_comparator.hpp"
#include "../../pic_loader/extent.hpp"
#include "../lua_lazy_cache.hpp"



namespace elona
{

struct PortraitData
{
    SharedId id;
    SharedId key;
    Extent rect;
    optional<fs::path> filepath;
};



ELONA_DEFINE_LUA_DB(_PortraitDBBase, PortraitData, false, "core.portrait")



class PortraitDB : public _PortraitDBBase
{
public:
    void load_all();


    /**
     * Takes a non-fully-quallified ID, and returns the next
     * non-fully-quallified ID. If `current` does not exist in the storage,
     * returns `current` itself. Portrait IDs are sorted in lexicographical
     * order (depending on `operator<()` for two `std::string`s).
     */
    std::string get_next_portrait(const std::string& current);

    /**
     * Takes a non-fully-quallified ID, and returns the previous
     * non-fully-quallified ID. If `current` does not exist in the storage,
     * returns `current` itself. Portrait IDs are sorted in lexicographical
     * order (depending on `operator<()` for two `std::string`s).
     */
    std::string get_previous_portrait(const std::string& current);


private:
    std::set<std::string, lib::natural_order_comparator> sorted_portrait_table;

    void _cache_sorted_portrait_table();
};



extern PortraitDB the_portrait_db;

} // namespace elona
