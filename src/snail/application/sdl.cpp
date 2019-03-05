#ifdef ANDROID
#include "../application.hpp"
#endif
#include <algorithm>
#include <map>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "../hsp.hpp"
#include "../input.hpp"
#include "../touch_input.hpp"


namespace elona
{
namespace snail
{

Application::Application()
    : _sdl_core(new detail::SDLCore)
    , _sdl_ttf(new detail::SDLTTF)
    , _sdl_image(new detail::SDLImage)
    , _sdl_mixer(new detail::SDLMixer)
{
}



Application& Application::instance()
{
    static Application the_instance;
    return the_instance;
}


void Application::set_title(const std::string& title)
{
    _title = title;
    if (_window)
    {
        ::SDL_SetWindowTitle(_window->ptr(), title.c_str());
    }
}


void Application::initialize_dpi()
{
    const constexpr int display_in_use =
        0; // Assume the first display is being used.
    const constexpr float default_dpi =
#ifdef __APPLE__
        72.0f;
#else
        96.0f;
#endif

    if (::SDL_GetDisplayDPI(display_in_use, NULL, &_dpi, NULL) != 0)
    {
        _dpi = default_dpi;
    }
}


void Application::initialize(const std::string& title)
{
    _width = 800;
    _height = 600;
    _physical_width = _width;
    _physical_height = _height;
    _title = title;
    _window.reset(new Window(
        title,
        Window::InitialPosition::undefined,
        Window::InitialPosition::undefined,
        _width,
        _height,
        Window::Flag::shown));
    _renderer.reset(new Renderer(
        *_window, Renderer::Flag::accelerated | Renderer::Flag::present_vsync));

    initialize_dpi();

    set_display_mode(get_default_display_mode());

    if (!is_android)
    {
        ::SDL_StartTextInput();
    }
}



void Application::quit()
{
    _will_quit = true;
}



void Application::register_finalizer(std::function<void()> finalizer)
{
    _finalizers.emplace_back(finalizer);
}



void Application::handle_event(const ::SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_QUIT: quit(); break;
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        Input::instance()._handle_event(event.button);
        break;
    case SDL_KEYUP:
    case SDL_KEYDOWN: Input::instance()._handle_event(event.key); break;
    case SDL_TEXTINPUT: Input::instance()._handle_event(event.text); break;
    case SDL_TEXTEDITING: Input::instance()._handle_event(event.edit); break;
    case SDL_FINGERMOTION:
    case SDL_FINGERDOWN:
    case SDL_FINGERUP: Input::instance()._handle_event(event.tfinger); break;
    case SDL_WINDOWEVENT: handle_window_event(event.window); break;
    default: break;
    }
}



void Application::on_size_changed(const ::SDL_WindowEvent& event)
{
    int new_width = event.data1;
    int new_height = event.data2;

    _physical_width = new_width;
    _physical_height = new_height;

    if (!is_android)
    {
        _width = new_width;
        _height = new_height;
    }

    update_orientation();

    if (is_android)
    {
        TouchInput::instance().initialize_quick_actions();
    }
}



void Application::handle_window_event(const ::SDL_WindowEvent& event)
{
    switch (event.event)
    {
    case SDL_WINDOWEVENT_FOCUS_LOST: _focus_lost_just_now = true; break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        // Handle device rotation.
        on_size_changed(event);
        break;
    case SDL_WINDOWEVENT_SHOWN:
    case SDL_WINDOWEVENT_EXPOSED:
    case SDL_WINDOWEVENT_RESTORED:
        if (_call_redraw)
        {
            hsp::redraw();
        }
        break;
    default: break;
    }
}



void Application::proc_event()
{
    ::SDL_Event event;
    while (::SDL_PollEvent(&event))
    {
        handle_event(event);
    }

    Input::instance()._update();

    if (_will_quit)
    {
        std::exit(0);
    }
}



void Application::wait(size_t msec)
{
    const auto now = ::SDL_GetTicks();
    if (_last_wait_time == 0)
    {
        _last_wait_time = now;
    }
    const auto delta = now - _last_wait_time;
    if (msec > delta)
    {
        ::SDL_Delay(msec - delta);
    }
    _last_wait_time = now;
}



void Application::set_fullscreen_mode(Window::FullscreenMode fullscreen_mode)
{
    (*_window).set_fullscreen_mode(fullscreen_mode);
    _fullscreen_mode = fullscreen_mode;
}



std::map<std::string, ::SDL_DisplayMode> Application::get_display_modes()
{
    const constexpr int display_in_use =
        0; // Assume the first display is being used.
    std::map<std::string, ::SDL_DisplayMode> display_modes;

    int display_mode_count = ::SDL_GetNumDisplayModes(display_in_use);
    if (display_mode_count < 1)
    {
        throw detail::SDLError("No display modes available");
    }

    for (int i = 0; i < display_mode_count; ++i)
    {
        SDL_DisplayMode mode;
        detail::enforce_sdl(::SDL_GetDisplayMode(display_in_use, i, &mode));

        if (mode.w < 800 || mode.h < 600)
        {
            continue;
        }

        std::stringstream ss;
        ss << mode.w << "x" << mode.h << "@" << mode.refresh_rate << "Hz";

        display_modes.emplace(ss.str(), mode);
    }

    return display_modes;
}

std::string Application::get_default_display_mode()
{
    auto display_modes = get_display_modes();
    if (display_modes.size() == 0)
    {
        throw detail::SDLError("No display modes available");
    }

    for (const auto pair : display_modes)
    {
        if (pair.second.w == 800 && pair.second.h == 600)
        {
            return pair.first;
        }
    }

    return display_modes.begin()->first;
}



::SDL_DisplayMode Application::get_display_mode()
{
    return (*_window).get_display_mode();
}



void Application::set_display_mode(const std::string& display_mode_str)
{
    std::string display_mode = display_mode_str;
    auto display_modes = get_display_modes();
    if (display_modes.size() == 0)
    {
        throw detail::SDLError("No display modes available");
    }
    if (display_modes.find(display_mode_str) == display_modes.end())
    {
        display_mode = get_default_display_mode();
    }

    set_display_mode(display_modes[display_mode]);
}

void Application::set_display_mode(::SDL_DisplayMode display_mode)
{
    if (is_fullscreen())
    {
        _window->set_display_mode(display_mode);
    }
    else
    {
        _window->set_size(display_mode.w, display_mode.h);
    }

    // Keep the actual rendered size separate from the device's size
    // on Android.
    _physical_width = display_mode.w;
    _physical_height = display_mode.h;

    if (!is_android)
    {
        _width = display_mode.w;
        _height = display_mode.h;
    }

    _window->move_to_center();

    update_orientation();
}

void Application::set_subwindow_display_mode(const std::string& mode)
{
    size_t found;

    // Parse a string like "800x600" for width and height
    if ((found = mode.find("x")) != std::string::npos)
    {
        std::string width_s = mode.substr(0, found);
        std::string height_s = mode.substr(found + 1);

        try
        {
            int width = boost::lexical_cast<int>(width_s);
            int height = boost::lexical_cast<int>(height_s);

            if (width < 800 || height < 600)
            {
                throw std::logic_error(
                    "Subwindow resolution too small: " + mode);
            }

            _width = width;
            _height = height;
        }
        catch (const boost::bad_lexical_cast&)
        {
            throw std::logic_error("Invalid subwindow mode string: " + mode);
        }
    }
    else
    {
        throw std::logic_error("Invalid subwindow mode string: " + mode);
    }
}

void Application::update_orientation()
{
    if (_physical_width < _physical_height)
    {
        _orientation = Orientation::portrait;
    }
    else
    {
        _orientation = Orientation::landscape;
    }

    _window_pos = calculate_android_window_pos();
}

static Rect calculate_android_window_pos_portrait(
    int window_width,
    int window_height,
    int physical_width)
{
    // Put the output window at the top of the screen, so the
    // touch controls have space at the bottom.
    float ratio = (float)physical_width / (float)window_width;
    int height = (int)((float)window_height * ratio);

    return {0, 0, physical_width, height};
}

static Rect calculate_android_window_pos_landscape(
    int window_width,
    int window_height,
    int physical_width,
    int physical_height)
{
    int x, y, width, height;

    // Fit the output window to the screen.
    float ww = (float)window_width;
    float wh = (float)window_height;
    float pw = (float)physical_width;
    float ph = (float)physical_height;

    // Does the new window height exceed the physical screen height?
    bool exceeds_physical_height = (ww / wh) <= (pw / ph);

    if (ww > wh && !exceeds_physical_height)
    {
        width = physical_width;
        height = (int)((wh * (float)width) / ww);
        x = 0;
        y = (physical_height - height) / 2;
    }
    else
    {
        height = physical_height;
        width = (int)((ww * (float)height) / wh);
        y = 0;
        x = (physical_width - width) / 2;
    }

    width = std::min(width, physical_width);
    height = std::min(height, physical_height);
    x = std::max(x, 0);
    y = std::max(y, 0);

    return {x, y, width, height};
}

Rect Application::calculate_android_window_pos()
{
    if (_orientation == Orientation::portrait)
    {
        return calculate_android_window_pos_portrait(
            _width, _height, _physical_width);
    }
    else
    {
        return calculate_android_window_pos_landscape(
            _width, _height, _physical_width, _physical_height);
    }
}



void Application::set_window_size(int width, int height)
{
    _window->set_size(width, height);
}

} // namespace snail
} // namespace elona
