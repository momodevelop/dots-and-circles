/* date = May 22nd 2021 7:14 pm */

#ifndef GAME_MODE_MAIN_DEBUG_H
#define GAME_MODE_MAIN_DEBUG_H

static inline void 
Main_RenderDebugLines(game_mode_main* Mode){
    circle2f Circle = {};
    Circle.origin = Mode->Player.Position;
    Circle.radius = Mode->Player.HitCircle.radius;
    Renderer_DrawCircle2f(G_Renderer, Circle, 1.f, 8, C4F_GREEN, ZLayDebug);
}

#endif //GAME_MODE_MAIN_DEBUG_H
