#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_bullet {
    MM_V2f Position;
    MM_Circle2f HitCircle;
    b32 IsHit;
    MM_V2f Velocity;
};
struct game_mode_sandbox {
    game_mode_sandbox_bullet Bullets[2500];
    game_camera Camera;
    
    MM_V2f PrevMousePos;
    MM_V2f CurMousePos; 
    u32 ClickCount;
    
    f32 PlayerCircleRadius;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = MM_V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = MM_V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
        Mode->Camera.Dimensions = MM_V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    
    const f32 BulletRadius = 8.f;
    f32 StartX = Game_DesignWidth * -0.5f + BulletRadius;
    f32 StartY = Game_DesignHeight * -0.5f + BulletRadius;
    f32 OffsetX = 0.f;
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        B->Position = MM_V2f_Create(StartX + OffsetX, StartY + OffsetY);
        B->HitCircle = MM_Circle2f_Create({}, BulletRadius);
        
        OffsetX += BulletRadius * 2; // padding
        if(OffsetX >= Game_DesignWidth) {
            OffsetX = 0.f;
            OffsetY += BulletRadius * 2;
        }
        B->Velocity = MM_V2f_Create(0.f, 0.f);
    }
    
    Mode->ClickCount = 0;
    Mode->PlayerCircleRadius = 16.f;
    
}


static inline void
UpdateSandboxMode(permanent_state* PermState, 
                  transient_state* TranState,
                  mailbox* RenderCommands, 
                  platform_input* Input,
                  f32 DeltaTime) 
{
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    assets* Assets = &TranState->Assets;
    
    // NOTE(Momo): Update
    if (Button_IsPoked(Input->ButtonSwitch)) {
        Mode->PrevMousePos = Mode->CurMousePos;
        Mode->CurMousePos = Camera_ScreenToWorld(&Mode->Camera, 
                                                 Input->DesignMousePos);
        ++Mode->ClickCount;
    }
    
    // NOTE(Momo): Update Bullets
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        
        MM_V2f TimedVelocity = MM_V2f_Mul(B->Velocity, DeltaTime);
        B->Position =  MM_V2f_Add(B->Position, TimedVelocity);
        
    }
    
    
    // NOTE(Momo): Line to circle collision
    MM_Line2f BonkLine = Line2f_CreateFromV2f(Mode->PrevMousePos, Mode->CurMousePos);
    if (Mode->ClickCount >= 2) {
        if (!MM_V2f_IsEqual(Mode->PrevMousePos, Mode->CurMousePos)) {
            
            for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
                game_mode_sandbox_bullet* B = Mode->Bullets + I;
                if (B->IsHit) {
                    continue;
                }
                MM_Circle2f C = B->HitCircle;
                C.origin = B->Position;
                
                MM_Circle2f PC = MM_Circle2f_Create(Mode->PrevMousePos,
                                              Mode->PlayerCircleRadius);
                MM_V2f PCVel = MM_V2f_Sub(Mode->CurMousePos, Mode->PrevMousePos);
                MM_V2f TimedVelocity = MM_V2f_Mul(B->Velocity, DeltaTime);
                
                if (Bonk2_IsDynaCircleXDynaCircle(PC, PCVel, C, TimedVelocity)) {
                    B->IsHit = True;
                }
                
            }
        }
    }
    
    // NOTE(Momo): Render
    Camera_Set(&Mode->Camera, RenderCommands);
    
    // NOTE(Momo): Render Bullets
    f32 ZOrder = 0.f;
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        MM_M44f S = MM_M44f_Scale(B->HitCircle.radius*2, B->HitCircle.radius*2, 1.f);
        MM_M44f T = MM_M44f_Translation(B->Position.x,
                                  B->Position.y,
                                  ZOrder += 0.001f);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                       Assets,
                                       Image_BulletDot,
                                       MM_M44f_Concat(T,S), 
                                       c4f{1.f, 1.f, 1.f, 0.5f});
    }
    
    // NOTE(Momo) Render Lines
    if (Mode->ClickCount >= 2) {
        ZOrder = 10.f;
        Renderer_DrawLine2f(RenderCommands, 
                            BonkLine,
                            Mode->PlayerCircleRadius * 2,
                            Color_Green,
                            ZOrder);
        
        MM_Circle2f StartCircle = MM_Circle2f_Create(BonkLine.min, 
                                               Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(RenderCommands,
                              StartCircle,
                              1.f, 
                              8, 
                              Color_Green, 
                              ZOrder);
        
        
        MM_Circle2f EndCircle = MM_Circle2f_Create(BonkLine.max, 
                                             Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(RenderCommands,
                              EndCircle,
                              1.f, 
                              8, 
                              Color_Green, 
                              ZOrder);
    }
    
}

#endif 
