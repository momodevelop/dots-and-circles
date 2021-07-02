
static inline void
Main_UpdateInput(game_mode_main* mode)
{
    Player* player = &mode->player; 
    
    v2f direction = {};
    
    player->prev_position = player->position;
    player->position = mode->camera.screen_to_world(G_Input->design_mouse_pos);
    
    
    // NOTE(Momo): Absorb mode Switch
    if(G_Input->button_switch.is_poked()) {
        player->mood_type = 
            (player->mood_type == MOOD_TYPE_DOT) ? MOOD_TYPE_CIRCLE : MOOD_TYPE_DOT;
        
        switch(player->mood_type) {
            case MOOD_TYPE_DOT: {
                player->dot_image_alpha_target = 1.f;
            } break;
            case MOOD_TYPE_CIRCLE: {
                player->dot_image_alpha_target = 0.f;
            }break;
            default:{ 
                ASSERT(false);
            }
        }
        player->dot_image_transition_timer = 0.f;
    }
}


static inline void 
Main_UpdatePlayer(game_mode_main* mode, 
                  f32 dt) 
{
    Player* player = &mode->player; 
    player->dot_image_alpha = LERP(1.f - player->dot_image_alpha_target, 
                                   player->dot_image_alpha_target, 
                                   player->dot_image_transition_timer / player->dot_image_transition_duration);
    
    player->dot_image_transition_timer += dt;
    player->dot_image_transition_timer = 
        CLAMP(player->dot_image_transition_timer, 
              0.f, 
              player->dot_image_transition_duration);
    
}

static inline void
Main_RenderScore(game_mode_main* Mode) {
    static constexpr f32 FontSize = 24.f;
    
    // NOTE(Momo): Current Score
    {
        draw_text(FONT_DEFAULT, 
                  { -Game_DesignWidth * 0.5f + 10.f, 
                      Game_DesignHeight * 0.5f - 24.f,  
                      ZLayScore },
                  String::create("Current Score"),
                  FontSize);
    }
    
    // NOTE(Momo): High Score
    {
        String Str = String::create("High Score");
        draw_text(FONT_DEFAULT,
                  { (Game_DesignWidth * 0.5f) - 120.f,
                      Game_DesignHeight * 0.5f - 24.f,
                      ZLayScore },
                  Str,
                  FontSize);
    }
    
}


static inline void 
Main_RenderPlayer(game_mode_main* mode) 
{
    Player* player = &mode->player;
    m44f s = m44f::create_scale(player->size, player->size, 1.f);
    
    {
        m44f t = m44f::create_translation(player->position.x,
                                          player->position.y,
                                          ZLayPlayer);
        c4f color = c4f::create(1.f, 1.f, 1.f, 1.f - player->dot_image_alpha);
        
        draw_textured_quad_from_image(IMAGE_PLAYER_CIRCLE,
                                      t*s, 
                                      color);
    }
    
    {
        m44f t = m44f::create_translation(player->position.x,
                                          player->position.y,
                                          ZLayPlayer + 0.01f);
        c4f color = c4f::create(1.f, 1.f, 1.f, player->dot_image_alpha);
        draw_textured_quad_from_image(IMAGE_PLAYER_DOT,
                                      t*s, 
                                      color);
    }
}
