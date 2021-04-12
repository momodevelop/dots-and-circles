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
Camera_Set(game_camera* C, mailbox* RenderCommands) {
    aabb3f CenterBox = Aabb3f_CreateCentered(C->Dimensions, C->Anchor);
    Renderer_ClearColor(RenderCommands, C->Color);
    Renderer_SetOrthoCamera(RenderCommands, 
                            C->Position, 
                            CenterBox);
}

static inline void
Camera_ScreenToWorld(game_camera* C) {
    // TODO(Momo): 
    
}

#endif //GAME_CAMERA_H
