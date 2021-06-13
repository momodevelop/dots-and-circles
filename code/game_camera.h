/* date = April 12th 2021 6:58 pm */

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

// NOTE(Momo): "2D" Orthographic camera. 
// Saves the current camera state.
// No direction because we are assuming 2D
struct game_camera {
    v3f Position;
    v3f Anchor; 
    v3f Dimensions; // In 2D, this represents the near and far clip planes
    c4f Color;
    
};

static inline void
Camera_Set(game_camera* C) {
    aabb3f CenterBox = Aabb3f_CreateCentered(C->Dimensions, C->Anchor);
    Renderer_ClearColor(G_Renderer, C->Color);
    Renderer_SetOrthoCamera(G_Renderer, 
                            C->Position, 
                            CenterBox);
}

static inline v2f
Camera_ScreenToView(game_camera* C, v2f ScreenPos) {
    v2f Ret = {};
    Ret.X = ScreenPos.X - C->Dimensions.W * 0.5f;
    Ret.Y = -(ScreenPos.Y - C->Dimensions.H * 0.5f);
    
    return Ret;
}

// TODO(Momo): Largely untested!!
static inline v2f
Camera_ScreenToWorld(game_camera* C, v2f ScreenPos) {
    
    v2f Ret = Camera_ScreenToView(C, ScreenPos);
    Ret.X -= C->Position.X;
    Ret.Y -= C->Position.Y;
    
    
    return Ret;
}

#endif //GAME_CAMERA_H
