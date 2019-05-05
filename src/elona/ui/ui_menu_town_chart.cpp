#include "ui_menu_town_chart.hpp"
#include "../area.hpp"
#include "../audio.hpp"
#include "../draw.hpp"
#include "../i18n.hpp"

namespace elona
{
namespace ui
{

bool UIMenuTownChart::init()
{
    return true;
}

void UIMenuTownChart::update()
{
    listmax = 0;
    page = 0;
    pagesize = 16;
    cs = 0;
    cc = 0;
    cs_bk = -1;
    snd("core.chat");
    asset_load("deco_politics");
    gsel(0);
    draw_bg("deco_politics_a");
    render_hud();
    windowshadow = 1;

    _city = area_data[game_data.current_map].quest_town_id;
    lv = 0;

    cs_bk = -1;
    pagemax = (listmax - 1) / pagesize;
    if (page < 0)
    {
        page = pagemax;
    }
    else if (page > pagemax)
    {
        page = 0;
    }
}

void UIMenuTownChart::draw()
{
    ui_display_window(
        i18n::s.get("core.locale.ui.town_chart.title"),
        strhint3b,
        (windoww - 580) / 2 + inf_screenx,
        winposy(400),
        580,
        400);
    keyrange = 0;
    int j0 = 0;
    int n = 0;
    cs_listbk();
    if (area_data[game_data.current_map].quest_town_id == 0 ||
        game_data.current_dungeon_level != 1)
    {
        font(14 - en * 2);
        mes(wx + 40,
            wy + 50,
            i18n::s.get("core.locale.ui.town_chart.no_economy"));
    }
    else
    {
        display_topic(
            i18n::s.get(
                "core.locale.ui.town_chart.chart",
                mapname(area_data[game_data.current_map].id)),
            wx + 40,
            wy + 34);
        for (int cnt = 0;; ++cnt)
        {
            if (pochart(j0, n, lv) == 0 || cnt == 0)
            {
                if (cnt != 0)
                {
                    ++n;
                }
                j0 = 0;
                i = 0;
                for (int cnt = 0; cnt < 10; ++cnt)
                {
                    if (pochart(cnt, n, lv) != 0)
                    {
                        ++i;
                    }
                }
                if (i == 0)
                {
                    break;
                }
                y = wy + 70 + n * 55;
            }
            x = wx + (ww - 70) / (i + 1) * (j0 + 1);
            elona::draw("deco_politics_b", x - 26, y - 3);
            p = pochart(j0, n, lv);
            key_list(cnt) = key_select(cnt);
            ++keyrange;
            display_key(x - 30, y + 21, cnt);
            font(12 + sizefix - en * 2);
            bmes(
                i18n::s.get_enum("core.locale.politics.post", p),
                x - 2,
                y + jp * 2);
            font(14 - en * 2);
            if (podata(0 + cnt, _city) == 0)
            {
                s = i18n::s.get("core.locale.ui.town_chart.empty");
            }
            cs_list(cs == cnt, s, x - 2, y + 20);
            ++j0;
        }
        if (keyrange != 0)
        {
            cs_bk = cs;
        }
    }
}

optional<UIMenuTownChart::ResultType> UIMenuTownChart::on_key(
    const std::string& action)
{
    if (get_selected_index_this_page())
    {
        return UIMenuTownChart::Result::finish();
    }
    else if (action == "next_page")
    {
        if (pagemax != 0)
        {
            snd("core.pop1");
            ++page;
            set_reupdate();
        }
    }
    else if (action == "previous_page")
    {
        if (pagemax != 0)
        {
            snd("core.pop1");
            --page;
            set_reupdate();
        }
    }
    else if (action == "cancel")
    {
        update_screen();
        return UIMenuTownChart::Result::cancel();
    }

    return none;
}

} // namespace ui
} // namespace elona
