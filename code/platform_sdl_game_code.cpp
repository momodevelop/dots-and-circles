#ifndef __PLATFORM_SDL_GAME_CODE__
#define __PLATFORM_SDL_GAME_CODE__

// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    game_update* Update;
};

static inline void
Unload(sdl_game_code* GameCode) {
    GameCode->Update = nullptr;
}

static inline bool
Load(sdl_game_code* GameCode)
{
    Unload(GameCode);
    
    void* GameCodeDLL = SDL_LoadObject("game.dll");
    if (!GameCodeDLL) {
        SDL_Log("Failed to open game.dll");
        return false;
    }
    
    GameCode->Update = (game_update*)SDL_LoadFunction(GameCodeDLL, "GameUpdate");
    
    return true;
}

#endif