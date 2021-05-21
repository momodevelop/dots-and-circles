/* date = April 12th 2021 6:58 pm */

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

// NOTE(Momo): "2D" Orthographic camera. 
// Saves the current camera state.
// No direction because we are assuming 2D
struct game_camera {
    MM_V3f Position;
    MM_V3f Anchor; 
    MM_V3f Dimensions; // In 2D, this represents the near and far clip planes
    c4f Color;
    
};

static inline void
Camera_Set(game_camera* C, mailbox* RenderCommands) {
    MM_Aabb3f CenterBox = MM_Aabb3f_CreateCentered(C->Dimensions, C->Anchor);
    Renderer_ClearColor(RenderCommands, C->Color);
    Renderer_SetOrthoCamera(RenderCommands, 
                            C->Position, 
                            CenterBox);
}

static inline MM_V2f
Camera_ScreenToView(game_camera* C, MM_V2f ScreenPos) {
    MM_V2f Ret = {};
    Ret.x = ScreenPos.x - C->Dimensions.W * 0.5f;
    Ret.y = -(ScreenPos.y - C->Dimensions.H * 0.5f);
    
    return Ret;
}

// TODO(Momo): Largely untested!!
static inline MM_V2f
Camera_ScreenToWorld(game_camera* C, MM_V2f ScreenPos) {
    
    MM_V2f Ret = Camera_ScreenToView(C, ScreenPos);
    Ret.x -= C->Position.x;
    Ret.y -= C->Position.y;
    
    
    return Ret;
}

#endif //GAME_CAMERA_H
