
static inline void
Main_UpdateInput(game_mode_main* Mode)
{
    v2f Direction = {};
    player* Player = &Mode->Player; 
    
    Player->PrevPosition = Player->Position;
    Player->Position = Camera_ScreenToWorld(&Mode->Camera,
                                            G_Input->DesignMousePos);
    
    
    // NOTE(Momo): Absorb Mode Switch
    if(Button_IsPoked(G_Input->ButtonSwitch)) {
        Player->MoodType = 
            (Player->MoodType == MoodType_Dot) ? MoodType_Circle : MoodType_Dot;
        
        switch(Player->MoodType) {
            case MoodType_Dot: {
                Player->DotImageAlphaTarget = 1.f;
            } break;
            case MoodType_Circle: {
                Player->DotImageAlphaTarget = 0.f;
            }break;
            default:{ 
                ASSERT(false);
            }
        }
        Player->DotImageTransitionTimer = 0.f;
    }
}


static inline void 
Main_UpdatePlayer(game_mode_main* Mode, 
                  f32 DeltaTime) 
{
    player* Player = &Mode->Player; 
    Player->DotImageAlpha = LERP(1.f - Player->DotImageAlphaTarget, 
                                 Player->DotImageAlphaTarget, 
                                 Player->DotImageTransitionTimer / Player->DotImageTransitionDuration);
    
    Player->DotImageTransitionTimer += DeltaTime;
    Player->DotImageTransitionTimer = 
        CLAMP(Player->DotImageTransitionTimer, 
              0.f, 
              Player->DotImageTransitionDuration);
    
}

static inline void
Main_RenderScore(game_mode_main* Mode) {
    static constexpr f32 FontSize = 24.f;
    
    // NOTE(Momo): Current Score
    {
        Draw_Text(FONT_DEFAULT, 
                  { -Game_DesignWidth * 0.5f + 10.f, 
                      Game_DesignHeight * 0.5f - 24.f,  
                      ZLayScore },
                  String::create("Current Score"),
                  FontSize);
    }
    
    // NOTE(Momo): High Score
    {
        String Str = String::create("High Score");
        Draw_Text(FONT_DEFAULT,
                  { (Game_DesignWidth * 0.5f) - 120.f,
                      Game_DesignHeight * 0.5f - 24.f,
                      ZLayScore },
                  Str,
                  FontSize);
    }
    
}


static inline void 
Main_RenderPlayer(game_mode_main* Mode) 
{
    player* Player = &Mode->Player;
    m44f S = m44f::create_scale(Player->Size, Player->Size, 1.f);
    
    {
        m44f T = m44f::create_translation(Player->Position.x,
                                          Player->Position.y,
                                          ZLayPlayer);
        c4f Color = c4f::create(1.f, 1.f, 1.f, 1.f - Player->DotImageAlpha);
        
        Draw_TexturedQuadFromImage(IMAGE_PLAYER_CIRCLE,
                                   T*S, 
                                   Color);
    }
    
    {
        m44f T = m44f::create_translation(Player->Position.x,
                                          Player->Position.y,
                                          ZLayPlayer + 0.01f);
        c4f Color = c4f::create(1.f, 1.f, 1.f, Player->DotImageAlpha);
        Draw_TexturedQuadFromImage(IMAGE_PLAYER_DOT,
                                   T*S, 
                                   Color);
    }
}
