#ifndef GAME_MODE_MAIN_CPP
#define GAME_MODE_MAIN_CPP


#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"
#include "game_mode_main_death_bomb.cpp"

// TODO: split state logic into files?
static inline b8 
Main_Init(permanent_state* PermState,
          transient_state* TranState,
          debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    arena* ModeArena = PermState->ModeArena;
    
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = C4f_Grey2;
        Mode->Camera.Dimensions = V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    b8 Success = false;
    
    Success = List_New(&Mode->DotBullets, ModeArena, DotCap);
    if (!Success) {
        return false;
    }
    
    Success = List_New(&Mode->CircleBullets, ModeArena, CircleCap);
    if (!Success) {
        return false;
    }
    
    Success = List_New(&Mode->Enemies, ModeArena, EnemyCap);
    if (!Success) {
        return false;
    }
    
    Success = Queue_New(&Mode->Particles, ModeArena, ParticleCap);
    if (!Success) {
        return false;
    }
    
    
    Success = BigInt_New(&Mode->Score, ModeArena, 128);
    if (!Success) {
        return false;
    }
    Mode->Wave.IsDone = true;
    Mode->Rng = Rng_Seed(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Position = {};
        Player->PrevPosition = {};
        Player->Size = Player->MaxSize;
        Player->HitCircle = { v2f{}, 16.f};
        
        // NOTE(Momo): We start as Dot
        Player->MoodType = MoodType_Dot;
        Player->DotImageAlpha = 1.f;
        Player->DotImageAlphaTarget = 1.f;
        
        Player->DotImageTransitionDuration = 0.1f;
        Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
        Player->IsDead = false;
    }
    Mode->Wave.IsDone = true;
    
#if 0    
    Success = AudioMixer_Play(&TranState->Mixer, Sound_Test, false, &Mode->BgmHandle);
    if (!Success) {
        return false;
        
    }
#endif
    
    
    Mode->State = Main_StateType_Spawning;
    return true; 
    
}

static inline void
Main_StateNormal_Update(permanent_state* PermState, 
                        transient_state* TranState,
                        debug_state* DebugState,
                        mailbox* RenderCommands, 
                        platform_input* Input,
                        f32 DeltaTime) 
{
    assets* Assets = &TranState->Assets;
    game_mode_main* Mode = PermState->MainMode;
    
    Camera_Set(&Mode->Camera, RenderCommands);
    
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    Main_UpdateBullets(Mode, DeltaTime);
    Main_UpdateWaves(Mode, Assets, DeltaTime);
    Main_UpdateEnemies(Mode, Assets, DeltaTime); 
    Main_UpdatePlayerBulletCollision(Mode, Assets, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    
    // NOTE(Momo): Render Score
    {
        arena_mark Scratch = Arena_Mark(&TranState->Arena);
        Defer { Arena_Revert(&Scratch); };
        
        
        u8_str Str = {};
        U8Str_New(&Str, Scratch, Mode->Score.Places);
        
        auto Itr = BigInt_ReverseItrBegin(&Mode->Score);
        auto End = BigInt_ReverseItrEnd(&Mode->Score);
        for (; Itr != End; ++Itr) {
            U8Str_Push(&Str, DigitToAscii((*Itr)));
        }
        
        // TODO: If we expose this part below, remember to check for Str.Count > 0
        
        // NOTE(Momo): We are just going to assume that:
        // - The text box boundary that is the game space is a square
        // - And that our numbers in font are monospaced
        const v2f Boundary = V2f_Create(Game_DesignWidth * 0.9f, 
                                        Game_DesignHeight * 0.9f);
        
        // NOTE(Momo): Get the font glyph's aabb
        font* Font = Assets_GetFont(Assets, Font_Default);
        font_glyph* BaseFontGlyph = Font_GetGlyph(Font, Str.Data[0]);
        v2f GlyphBox = V2f_Create(Aabb2f_Width(BaseFontGlyph->Box), 
                                  Aabb2f_Height(BaseFontGlyph->Box));
        
        // NOTE(Momo): This holds the value of BOTH the amount of glyphs
        // in each row and column
        u32 Rows = (u32)Sqrt((f32)Str.Count); // Truncation expected
        u32 ExtraCharCount = Str.Count - (Rows*Rows);
        
        // NOTE(Momo): At this point, 
        // Str.Count == SquareLen * SquareLen + Overflow
        
        f32 FontScaleToFitBoundaryH = Boundary.H/GlyphBox.H/Rows;
        f32 FontScaleToFitBoundaryW = Boundary.W/GlyphBox.W/Rows;
        f32 FontScaleToFitBoundaryBest = MinOf(FontScaleToFitBoundaryW, FontScaleToFitBoundaryH);
        
        f32 FontGlyphW = GlyphBox.W * FontScaleToFitBoundaryBest;
        f32 FontGlyphH = GlyphBox.H * FontScaleToFitBoundaryBest;
        
        c4f Color = C4f_Create(1,1,1, 0.05f);
        
#if 1
        for(auto Beg = BigInt_ReverseItrBegin(&Mode->Score); Beg != BigInt_ReverseItrEnd(&Mode->Score); ++Beg) {
            G_Platform->LogFp("%d", (*Beg));
        }
        G_Platform->LogFp("\n");
        G_Platform->LogFp("c: %d\n", Str.Count);
        G_Platform->LogFp("r: %d\n", Rows);
        G_Platform->LogFp("o: %d\n", ExtraCharCount);
        for (u32 RowIndex = 0; RowIndex < Rows; ++RowIndex ) {
            G_Platform->LogFp("r%d: %d + %d\n", RowIndex,
                              Rows + ExtraCharCount/Rows, (ExtraCharCount%Rows-1) >= RowIndex );
        }
#endif
        
#if 0
        u32 StringIndex = 0;
        for (u32 RowIndex = 0; RowIndex < SquareLen; ++RowIndex ) {
            u32 GrabFromOverflow = Overflow???/
                u32 GlyphsToRender = SquareLen + (Overflow-- ? 1 : 0);
            //G_Platform->LogFp("%d\n", GlyphsToRender);
            f32 StartPosX = FontGlyphW * GlyphsToRender * -0.5f + FontGlyphW * 0.5f;
            for (u32 I = 0; I < GlyphsToRender; ++I) {
                font_glyph* FontGlyph = Font_GetGlyph(Font, Str.Data[StringIndex]);
                m44f S = M44f_Scale(FontGlyphW, FontGlyphH, 1.f);
                
                f32 PosX = StartPosX + I * FontGlyphW;
                f32 PosY = 0.f;
                m44f T = M44f_Translation(PosX, PosY, ZLayScore);
                
                
                Draw_TexturedQuadFromImage(RenderCommands,
                                           Assets,
                                           FontGlyph->ImageId,
                                           T*S,
                                           Color);
                ++StringIndex;
            }
        }
#endif
        
        
        
    }
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    
    // NOTE(Momo): if player's dead, do dead stuff
    if(Mode->Player.IsDead) 
    {
        // NOTE(Momo): Drop the death bomb
        Mode->DeathBomb.Radius = 0.f;
        Mode->DeathBomb.Position = Mode->Player.Position;
        
        Mode->State = Main_StateType_PlayerDied;
        Mode->Player.Position = V2f_Create(-1000.f, -1000.f);
    }
}



static inline void
Main_StatePlayerDied_Update(permanent_state* PermState, 
                            transient_state* TranState,
                            debug_state* DebugState,
                            mailbox* RenderCommands, 
                            platform_input* Input,
                            f32 DeltaTime) 
{
    // Everything stops
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    Main_UpdateDeathBomb(Mode, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    Camera_Set(&Mode->Camera, RenderCommands);
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    Main_RenderDeathBomb(Mode, RenderCommands);
    
    // NOTE: Change state if enemy and bullet count is 0
    if (Mode->DeathBomb.Radius >= Game_DesignWidth * 2.f) 
    {
        Mode->State = Main_StateType_Spawning;
        Mode->SpawningState.Timer = 0.f;
        Mode->Player.IsDead = false;
    }
}

static inline void
Main_StateSpawning_Update(permanent_state* PermState, 
                          transient_state* TranState,
                          debug_state* DebugState,
                          mailbox* RenderCommands, 
                          platform_input* Input,
                          f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    f32 Ease = EaseOutBounce(Clamp(Mode->SpawningState.Timer/Mode->SpawningState.Duration, 0.f, 1.f));
    Mode->Player.Size = Mode->Player.MaxSize * Ease;
    
    
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    
    Camera_Set(&Mode->Camera, RenderCommands);
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    
    if (Mode->SpawningState.Timer >= Mode->SpawningState.Duration) {
        Mode->State = Main_StateType_Normal;
        Mode->Player.Size = Mode->Player.MaxSize;
    }
    Mode->SpawningState.Timer += DeltaTime;
    
    
}

static inline void
Main_Update(permanent_state* PermState, 
            transient_state* TranState,
            debug_state* DebugState,
            mailbox* RenderCommands, 
            platform_input* Input,
            f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    switch(Mode->State) {
        case Main_StateType_Spawning: {
            Main_StateSpawning_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        } break;
        case Main_StateType_Normal: {
            Main_StateNormal_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        }break;
        case Main_StateType_PlayerDied: {
            Main_StatePlayerDied_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        } break;
        case Main_StateType_Cleanup: {
        } break;
        
    }
    
    //Main_RenderDebugLines(Mode, RenderCommands);
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Dots: ");
    DebugInspector_PushU32(&DebugState->Inspector,
                           Buffer,
                           Mode->DotBullets.Count);
    U8CStr_InitFromSiStr(&Buffer, "Circles: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->CircleBullets.Count);
    
    U8CStr_InitFromSiStr(&Buffer, "Bullets: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->DotBullets.Count + Mode->CircleBullets.Count);
    
    
    U8CStr_InitFromSiStr(&Buffer, "Enemies: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->Enemies.Count);
}

#endif //GAME_MODE_H
