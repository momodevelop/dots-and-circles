/* date = July 30th 2020 0:41 pm */

#ifndef GAME_MODE_MENU_H
#define GAME_MODE_MENU_H

#include "game.h"
#include "game_mode_splash.h"


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_menu {
    static constexpr u8 TypeId = 1;
};

static inline void 
InitMode(game_mode_menu* Mode, game_state* GameState) {
    Log("Menu state initialized!");
}

static inline void
UpdateMode(game_mode_menu* Mode,
           game_state* GameState, 
           commands* RenderCommands, 
           game_input* Input,
           f32 DeltaTime) 
{
    
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });;
    PushCommandSetOrthoBasis(RenderCommands, { 0.f, 0.f, 0.f }, { 1600.f, 900.f, 200.f });
}



#endif //GAME_MODE_MENU_H
