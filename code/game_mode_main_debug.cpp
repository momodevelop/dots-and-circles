/* date = May 22nd 2021 7:14 pm */

#ifndef GAME_MODE_MAIN_DEBUG_H
#define GAME_MODE_MAIN_DEBUG_H

static inline void 
Main_RenderDebugLines(game_mode_main* Mode, mailbox* RenderCommands){
    circle2f Circle = {};
    Circle.Origin = Mode->Player.Position;
    Circle.Radius = Mode->Player.HitCircle.Radius;
    Renderer_DrawCircle2f(RenderCommands, Circle, 1.f, 8, C4f_Green, ZLayDebug);
}

#endif //GAME_MODE_MAIN_DEBUG_H
