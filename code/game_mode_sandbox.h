#ifndef _GAME_MODE_SANDBOX_
#define _GAME_MODE_SANDBOX_


// NOTE(Momo): Mode /////////////////////////////////////////////
struct Game_Mode_Sandbox_Bullet {
    v2f position;
    circle2f hit_circle;
    b8 is_hit;
    v2f velocity;
};
struct Game_Mode_Sandbox {
    Game_Mode_Sandbox_Bullet bullets[2500];
    Camera camera;
    
    v2f prev_mouse_pos;
    v2f cur_mouse_pos; 
    u32 click_count;
    
    f32 player_circle_radius;
};

static inline void 
init_sandbox_mode(Permanent_State* perm_state) {
    g_platform->hide_cursor();
    
    Game_Mode_Sandbox* mode = perm_state->sandbox_mode;     
    // NOTE(Momo): Init camera
    {
        mode->camera.position = v3f_Create(0.f, 0.f, 0.f);
        mode->camera.anchor = v3f_Create(0.5f, 0.5f, 0.5f);
        mode->camera.color = C4F_GREY2;
        mode->camera.dimensions = v3f_Create(GAME_DESIGN_WIDTH,
                                             GAME_DESIGN_HEIGHT,
                                             Game_DesignDepth);
    }
    
    
    const f32 bullet_radius = 8.f;
    f32 start_x = GAME_DESIGN_WIDTH * -0.5f + bullet_radius;
    f32 start_y = GAME_DESIGN_HEIGHT * -0.5f + bullet_radius;
    f32 offset_x = 0.f;
    f32 offset_y = 0.f;
    for (u32 I = 0; I < ArrayCount(mode->bullets); ++I) {
        Game_Mode_Sandbox_Bullet* b = mode->bullets + I;
        b->position = v2f_Create(start_x + offset_x, start_y + offset_y);
        b->hit_circle = circle2f_Create({}, bullet_radius);
        
        offset_x += bullet_radius * 2; // padding
        if(offset_x >= GAME_DESIGN_WIDTH) {
            offset_x = 0.f;
            offset_y += bullet_radius * 2;
        }
        b->velocity = v2f_Create(0.f, 0.f);
    }
    
    mode->click_count = 0;
    mode->player_circle_radius = 16.f;
    
}


static inline void
update_sandbox_mode(Permanent_State* perm_state, 
                    Transient_State* tran_state,
                    f32 dt) 
{
    Game_Mode_Sandbox* mode = perm_state->sandbox_mode;     
    
    // NOTE(Momo): Update
    if (g_input->button_switch.is_poked()) {
        mode->prev_mouse_pos = mode->cur_mouse_pos;
        mode->cur_mouse_pos = Camera_ScreenToWorld(&mode->camera, g_input->design_mouse_pos);
        ++mode->click_count;
    }
    
    // NOTE(Momo): Update bullets
    for (u32 I = 0; I < ArrayCount(mode->bullets); ++I) {
        Game_Mode_Sandbox_Bullet* b = mode->bullets + I;
        if (b->is_hit) {
            continue;
        }
        
        b->position += b->velocity * dt;
        
    }
    
    
    // NOTE(Momo): Line to circle collision
    line2f bonk_line = line2f::create(mode->prev_mouse_pos, mode->cur_mouse_pos);
    if (mode->click_count >= 2) {
        if (!IsEqual(mode->prev_mouse_pos, mode->cur_mouse_pos)) {
            
            for (u32 i = 0; i < ArrayCount(mode->bullets); ++i) {
                Game_Mode_Sandbox_Bullet* b = mode->bullets + i;
                if (b->is_hit) {
                    continue;
                }
                circle2f c = b->hit_circle;
                c.origin = b->position;
                
                circle2f pc = circle2f_Create(mode->prev_mouse_pos,
                                              mode->player_circle_radius);
                v2f pc_vel = mode->cur_mouse_pos - mode->prev_mouse_pos;
                v2f timed_velocity = b->velocity * dt;
                
                if (Bonk2_IsDynaCircleXDynaCircle(pc, pc_vel, c, timed_velocity)) {
                    b->is_hit = true;
                }
                
            }
        }
    }
    
    // NOTE(Momo): Render
    Camera_Set(&mode->camera);
    
    // NOTE(Momo): Render bullets
    f32 z_order = 0.f;
    for (u32 I = 0; I < ArrayCount(mode->bullets); ++I) {
        Game_Mode_Sandbox_Bullet* b = mode->bullets + I;
        if (b->is_hit) {
            continue;
        }
        m44f S = m44f_Scale(b->hit_circle.radius*2, b->hit_circle.radius*2, 1.f);
        m44f T = m44f_Translation(b->position.x,
                                  b->position.y,
                                  z_order += 0.001f);
        
        DrawTexturedQuadFromImage(IMAGE_BULLET_DOT,
                                  T*S, 
                                  c4f{1.f, 1.f, 1.f, 0.5f});
    }
    
    // NOTE(Momo) Render Lines
    if (mode->click_count >= 2) {
        z_order = 10.f;
        Renderer_DrawLine(g_renderer, 
                            bonk_line,
                            mode->player_circle_radius * 2,
                            C4F_GREEN,
                            z_order);
        
        circle2f start_circle = circle2f_Create(bonk_line.min, 
                                                mode->player_circle_radius);
        Renderer_DrawCircle(g_renderer,
                              start_circle,
                              1.f, 
                              8, 
                              C4F_GREEN, 
                              z_order);
        
        
        circle2f end_circle = circle2f_Create(bonk_line.max, 
                                              mode->player_circle_radius);
        Renderer_DrawCircle(g_renderer,
                              end_circle,
                              1.f, 
                              8, 
                              C4F_GREEN, 
                              z_order);
    }
    
}

#endif 
