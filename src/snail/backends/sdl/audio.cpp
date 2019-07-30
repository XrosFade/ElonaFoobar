#include "../../application.hpp"
#include "../../detail/sdl.hpp"

#include <vector>



namespace
{

// Channels:
// 0-6: reserved for SEs used very frequently
// 7: temporary(default)
// 8-12: temporary
// 13-16: reserved for weather effect

constexpr int max_channels = 17;


std::vector<Mix_Chunk*> chunks;

Mix_Music* played_music = nullptr;

} // namespace



namespace elona
{
namespace snail
{
namespace audio
{


void set_position(int channel, short angle, unsigned char distance)
{
    ::Mix_SetPosition(channel, angle, distance);
}


int DSINIT()
{
    Mix_AllocateChannels(max_channels);
    chunks.resize(max_channels);
    Application::instance().register_finalizer([&]() {
        for (const auto& chunk : chunks)
        {
            if (chunk)
                ::Mix_FreeChunk(chunk);
        }
    });
    return 1;
}



void DSLOADFNAME(const std::string& filepath, int channel)
{
    if (auto chunk = chunks[channel])
        Mix_FreeChunk(chunk);

    auto chunk = snail::detail::enforce_mixer(Mix_LoadWAV(filepath.c_str()));
    chunks[channel] = chunk;
}



void DSPLAY(int channel, bool loop)
{
    Mix_PlayChannel(channel, chunks[channel], loop ? -1 : 0);
}



void DSSTOP(int channel)
{
    Mix_HaltChannel(channel);
}



void DSSETVOLUME(int channel, int volume)
{
    if (const auto chunk = chunks[channel])
    {
        Mix_VolumeChunk(chunk, volume);
    }
}



bool CHECKPLAY(int channel)
{
    return Mix_Playing(channel);
}



int DMINIT()
{
    Application::instance().register_finalizer([&]() {
        if (played_music)
            ::Mix_FreeMusic(played_music);
    });
    return 1;
}



void DMLOADFNAME(const std::string& filepath, int)
{
    // TODO: find why MIDI is marked "unsupported"
    if (Application::is_android)
    {
        return;
    }
    if (played_music)
        ::Mix_FreeMusic(played_music);

    played_music = snail::detail::enforce_mixer(Mix_LoadMUS(filepath.c_str()));
}



void DMPLAY(int loop, int)
{
    // TODO: find why MIDI is marked "unsupported"
    if (Application::is_android)
    {
        return;
    }
    detail::enforce_mixer(Mix_PlayMusic(played_music, loop ? -1 : 1));
}



void DMSTOP()
{
    ::Mix_HaltMusic();
    if (played_music)
    {
        ::Mix_FreeMusic(played_music);
        played_music = nullptr;
    }
}

} // namespace audio
} // namespace snail
} // namespace elona
