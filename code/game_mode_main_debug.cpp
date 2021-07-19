/* date = May 22nd 2021 7:14 pm */

#ifndef GAME_MODE_MAIN_DEBUG_H
#define GAME_MODE_MAIN_DEBUG_H

static inline void 
Main_RenderDebugLines(Game_Mode_Main* mode){
    circle2f circle = {};
    circle.origin = mode->player.position;
    circle.radius = mode->player.hit_circle.radius;
    Renderer_DrawCircle(g_renderer, circle, 1.f, 8, C4F_GREEN, ZLayDebug);
}

#endif //GAME_MODE_MAIN_DEBUG_H
