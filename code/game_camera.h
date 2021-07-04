/* date = April 12th 2021 6:58 pm */

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

// NOTE(Momo): "2D" Orthographic camera. 
// Saves the current camera state.
// No direction because we are assuming 2D
struct Game_Camera {
    v3f position;
    v3f anchor; 
    v3f dimensions; // In 2D, this represents the near and far clip planes
    c4f color;
    
    void set();
    v2f screen_to_view(v2f screen_pos);
    v2f screen_to_world(v2f screen_pos);
    
};

void
Game_Camera::set() {
    aabb3f center_box = aabb3f::create_centered(this->dimensions, this->anchor);
    Renderer_ClearColor(g_renderer, this->color);
    Renderer_SetOrthoCamera(g_renderer, 
                            this->position, 
                            center_box);
}

v2f
Game_Camera::screen_to_view(v2f screen_pos) {
    v2f ret = {};
    ret.x = screen_pos.x - this->dimensions.w * 0.5f;
    ret.y = -(screen_pos.y - this->dimensions.h * 0.5f);
    
    return ret;
}

// TODO(Momo): Largely untested!!
v2f
Game_Camera::screen_to_world(v2f screen_pos) {
    
    v2f ret = screen_to_view(screen_pos);
    ret.x -= this->position.x;
    ret.y -= this->position.y;
    
    
    return ret;
}

#endif //GAME_CAMERA_H
