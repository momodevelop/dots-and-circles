/* date = April 12th 2021 6:58 pm */

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

// NOTE(Momo): "2D" Orthographic camera. 
// Saves the current camera state.
// No direction because we are assuming 2D
struct Camera {
    v3f position;
    v3f anchor; 
    v3f dimensions; // In 2D, this represents the near and far clip planes
    c4f color;
};

static inline void
Camera_Set(Camera* c) {
    aabb3f center_box = aabb2u_CreateCentered(c->dimensions, c->anchor);
    Renderer_ClearColor(g_renderer, c->color);
    Renderer_SetOrthoCamera(g_renderer, 
                            c->position, 
                            center_box);
}

static inline v2f
Camera_ScreenToView(Camera* c, v2f screen_pos) {
    v2f ret = {};
    ret.x = screen_pos.x - c->dimensions.w * 0.5f;
    ret.y = -(screen_pos.y - c->dimensions.h * 0.5f);
    
    return ret;
}

// TODO(Momo): Largely untested!!
static inline v2f
Camera_ScreenToWorld(Camera* c, v2f screen_pos) {
    v2f ret = Camera_ScreenToView(c, screen_pos);
    ret.x -= c->position.x;
    ret.y -= c->position.y;
    
    return ret;
}

#endif //GAME_CAMERA_H
