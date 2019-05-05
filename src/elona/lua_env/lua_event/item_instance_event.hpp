#pragma once
#include "../../item.hpp"
#include "../interface.hpp"
#include "base_event.hpp"

namespace elona
{
namespace lua
{

struct ItemInstanceEvent : BaseEvent
{
public:
    ItemInstanceEvent(const char* id, const Item& item)
        : BaseEvent(id)
    {
        _item = lua::handle(item);
        _item_id = item.new_id();
    }

    sol::table make_event_table() const override
    {
        auto args = lua::create_table();
        args["item"] = _item;

        return args;
    };

    sol::table make_event_options() const override
    {
        auto opts = lua::create_table();
        opts["instances"] = lua::create_table(1, _item, 2, _item_id);

        return opts;
    }

protected:
    LuaItemHandle _item;
    std::string _item_id;
};

} // namespace lua
} // namespace elona
