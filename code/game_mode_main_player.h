/* date = May 22nd 2021 7:12 pm */

#ifndef GAME_MODE_MAIN_PLAYER_H
#define GAME_MODE_MAIN_PLAYER_H

static inline void 
UpdatePlayer(game_mode_main* Mode, 
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
RenderPlayer(game_mode_main* Mode,
             assets* Assets,
             mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    m44f S = M44f_Scale(Player->Size.X, Player->Size.Y, 1.f);
    
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

#endif //GAME_MODE_MAIN_PLAYER_H
