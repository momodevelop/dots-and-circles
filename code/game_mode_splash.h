#ifndef GAME_MODE_SPLASH_H
#define GAME_MODE_SPLASH_H

#include "game.h"

struct Splash_Image_Entity {
    v3f scale;
    v3f position;
    c4f colors;
    
    Image_ID texture_aabb;
    
    f32 cd_timer;
    f32 cd_duration;
    
    f32 timer;
    f32 duration;
    f32 start_x;
    f32 end_x;
    
    void update(f32 dt);
};

void
Splash_Image_Entity::update(f32 dt) 
{
    this->cd_timer += dt;
    if (this->cd_timer <= this->cd_duration) 
        return;
    
    // NOTE(Momo): Update
    f32 Ease = ease_out_bounce(CLAMP(this->timer/this->duration, 0.f, 1.f));
    
    this->position.x = this->start_x + (this->end_x - this->start_x) * Ease; 
    this->timer += dt;
    
    // NOTE(Momo): Render
    m44f T = m44f::create_translation(this->position.x,
                                      this->position.y,
                                      this->position.z);
    m44f S = m44f::create_scale(this->scale.x,
                                this->scale.y,
                                1.f);
    draw_textured_quad_from_image(this->texture_aabb,
                                  T*S,
                                  this->colors);
    
    
}

struct Splash_Blackout_Entity {
    v3f scale;
    v3f position;
    c4f colors;
    
    f32 cd_timer;
    f32 cd_duration;
    
    f32 timer;
    f32 duration;
    
    void update(f32 dt);
};

void
Splash_Blackout_Entity::update(f32 dt) 
{
    this->cd_timer += dt;
    if (this->cd_timer <= this->cd_duration) 
        return;
    
    this->colors.a = ease_in_sine(CLAMP(this->timer/this->duration, 0.f, 1.f));
    this->timer += dt;
    
    // NOTE(Momo): Render
    m44f T = m44f::create_translation(this->position.x,
                                      this->position.y,
                                      this->position.z);
    m44f S = m44f::create_scale(this->scale.x, this->scale.y, 1.f);
    
    Renderer_DrawQuad(g_renderer, this->colors, T*S);
    
}

struct Game_Mode_Splash {
    Splash_Image_Entity images[2];
    Splash_Blackout_Entity blackout;
};


static inline void
init_splash_mode(Permanent_State* perm_state) {
    Game_Mode_Splash* mode = perm_state->splash_mode;
    g_platform->show_cursor();
    
    // NOTE(Momo): Create entities
    {
        mode->images[0].position = v3f::create( 0.f, 0.f, 0.f );
        mode->images[0].scale = v3f::create(400.f, 400.f, 1.f);
        mode->images[0].colors = c4f::create(1.f, 1.f, 1.f, 1.f);
        mode->images[0].texture_aabb = IMAGE_RYOJI;
        mode->images[0].cd_timer = 0.f;
        mode->images[0].cd_duration = 1.f;
        mode->images[0].timer = 0.f;
        mode->images[0].duration = 2.f;
        mode->images[0].start_x = -1000.f;
        mode->images[0].end_x = -200.f;
        
        mode->images[1].position = {};
        mode->images[1].scale = v3f::create(400.f, 400.f, 1.f);
        mode->images[1].colors = c4f::create(1.f, 1.f, 1.f, 1.f);
        mode->images[1].texture_aabb = IMAGE_YUU;
        mode->images[1].cd_timer = 0.f;
        mode->images[1].cd_duration = 1.f;
        mode->images[1].timer = 0.f;
        mode->images[1].duration = 2.f;
        mode->images[1].start_x = 1000.f;
        mode->images[1].end_x = 200.f;
        
        mode->blackout.position = v3f::create(0.f, 0.f, 1.0f);
        mode->blackout.scale = v3f::create(1600.f, 900.f, 1.f);
        mode->blackout.colors = c4f::create(0.f, 0.f, 0.f, 0.0f);
        mode->blackout.cd_timer = 0.f;
        mode->blackout.cd_duration = 3.f;
        mode->blackout.timer = 0.f;
        mode->blackout.duration = 1.f;
        
    }
}

static inline void
update_splash_mode(Permanent_State* perm_state,
                   Transient_State* tran_state,
                   f32 dt)
{
    Game_Mode_Splash* mode = perm_state->splash_mode;
    
    // NOTE: Camera
    {
        v3f position = v3f::create(0.f, 0.f, 0.f);
        v3f anchor = v3f::create(0.5f, 0.5f, 0.5f);
        c4f color = c4f::create(0.f, 0.3f, 0.3f, 1.f);
        v3f dimensions = v3f::create(800.f,
                                     800.f,
                                     800.f);
        
        Renderer_ClearColor(g_renderer, color);
        
        aabb3f frustum = aabb3f::create_centered(dimensions, anchor);
        Renderer_SetOrthoCamera(g_renderer, 
                                position, 
                                frustum);
    }
    
    for (u32 i = 0; i < ARRAY_COUNT(mode->images); ++i) {
        mode->images[i].update(dt);
    }
    
    mode->blackout.update(dt);
    
    // NOTE(Momo): Exit 
    if (mode->blackout.timer >= mode->blackout.duration) {
        perm_state->next_game_mode = GAME_MODE_MAIN;
    }
    
}

#endif //GAME_MODE_SPLASH_H
