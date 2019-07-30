#pragma once
#include "ui_menu.hpp"

namespace elona
{
namespace ui
{

class UIMenuCharamakeClass : public UIMenu<std::string>
{
public:
    UIMenuCharamakeClass(const std::string& race)
        : _race(race)
    {
    }

protected:
    virtual bool init();
    virtual void update();
    virtual void draw();
    virtual optional<UIMenuCharamakeClass::ResultType> on_key(
        const std::string& key);

private:
    void _draw_window();
    void _draw_choice(int, const std::string&);
    void _draw_choices();

    std::string _race;
};

} // namespace ui
} // namespace elona
