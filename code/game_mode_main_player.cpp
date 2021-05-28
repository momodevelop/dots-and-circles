
static inline void
Main_UpdateInput(game_mode_main* Mode,
                 platform_input* Input)
{
    v2f Direction = {};
    player* Player = &Mode->Player; 
    
    Player->PrevPosition = Player->Position;
    Player->Position = Camera_ScreenToWorld(&Mode->Camera,
                                            Input->DesignMousePos);
    
    
    // NOTE(Momo): Absorb Mode Switch
    if(Button_IsPoked(Input->ButtonSwitch)) {
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
                Assert(false);
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
    Player->DotImageAlpha = Lerp(1.f - Player->DotImageAlphaTarget, 
                                 Player->DotImageAlphaTarget, 
                                 Player->DotImageTransitionTimer / Player->DotImageTransitionDuration);
    
    Player->DotImageTransitionTimer += DeltaTime;
    Player->DotImageTransitionTimer = 
        Clamp(Player->DotImageTransitionTimer, 
              0.f, 
              Player->DotImageTransitionDuration);
    
}

static inline void 
Main_RenderPlayer(game_mode_main* Mode,
                  assets* Assets,
                  mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    m44f S = M44f_Scale(Player->Size, Player->Size, 1.f);
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, 1.f - Player->DotImageAlpha);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerCircle,
                                   M44f_Concat(T,S), 
                                   Color);
    }
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer + 0.01f);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, Player->DotImageAlpha);
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerDot,
                                   M44f_Concat(T,S), 
                                   Color);
    }
}
