#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_bullet {
    v2f Position;
    circle2f HitCircle;
    b8 IsHit;
    v2f Velocity;
};
struct game_mode_sandbox {
    game_mode_sandbox_bullet Bullets[2500];
    game_camera Camera;
    
    v2f PrevMousePos;
    v2f CurMousePos; 
    u32 ClickCount;
    
    f32 PlayerCircleRadius;
};

static inline void 
SandboxMode_Init(permanent_state* PermState) {
    G_Platform->HideCursorFp();
    
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = C4f_Grey2;
        Mode->Camera.Dimensions = V3f_Create(Game_DesignWidth,
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
        B->Position = V2f_Create(StartX + OffsetX, StartY + OffsetY);
        B->HitCircle = Circle2f_Create({}, BulletRadius);
        
        OffsetX += BulletRadius * 2; // padding
        if(OffsetX >= Game_DesignWidth) {
            OffsetX = 0.f;
            OffsetY += BulletRadius * 2;
        }
        B->Velocity = V2f_Create(0.f, 0.f);
    }
    
    Mode->ClickCount = 0;
    Mode->PlayerCircleRadius = 16.f;
    
}


static inline void
SandboxMode_Update(permanent_state* PermState, 
                   transient_state* TranState,
                   f32 DeltaTime) 
{
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    assets* Assets = &TranState->Assets;
    
    // NOTE(Momo): Update
    if (Button_IsPoked(G_Input->ButtonSwitch)) {
        Mode->PrevMousePos = Mode->CurMousePos;
        Mode->CurMousePos = Camera_ScreenToWorld(&Mode->Camera, 
                                                 G_Input->DesignMousePos);
        ++Mode->ClickCount;
    }
    
    // NOTE(Momo): Update Bullets
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        
        v2f TimedVelocity = V2f_Mul(B->Velocity, DeltaTime);
        B->Position =  V2f_Add(B->Position, TimedVelocity);
        
    }
    
    
    // NOTE(Momo): Line to circle collision
    line2f BonkLine = Line2f_CreateFromV2f(Mode->PrevMousePos, Mode->CurMousePos);
    if (Mode->ClickCount >= 2) {
        if (!V2f_IsEqual(Mode->PrevMousePos, Mode->CurMousePos)) {
            
            for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
                game_mode_sandbox_bullet* B = Mode->Bullets + I;
                if (B->IsHit) {
                    continue;
                }
                circle2f C = B->HitCircle;
                C.Origin = B->Position;
                
                circle2f PC = Circle2f_Create(Mode->PrevMousePos,
                                              Mode->PlayerCircleRadius);
                v2f PCVel = V2f_Sub(Mode->CurMousePos, Mode->PrevMousePos);
                v2f TimedVelocity = V2f_Mul(B->Velocity, DeltaTime);
                
                if (Bonk2_IsDynaCircleXDynaCircle(PC, PCVel, C, TimedVelocity)) {
                    B->IsHit = true;
                }
                
            }
        }
    }
    
    // NOTE(Momo): Render
    Camera_Set(&Mode->Camera);
    
    // NOTE(Momo): Render Bullets
    f32 ZOrder = 0.f;
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        m44f S = M44f_Scale(B->HitCircle.Radius*2, B->HitCircle.Radius*2, 1.f);
        m44f T = M44f_Translation(B->Position.X,
                                  B->Position.Y,
                                  ZOrder += 0.001f);
        
        Draw_TexturedQuadFromImage(Image_BulletDot,
                                   M44f_Concat(T,S), 
                                   c4f{1.f, 1.f, 1.f, 0.5f});
    }
    
    // NOTE(Momo) Render Lines
    if (Mode->ClickCount >= 2) {
        ZOrder = 10.f;
        Renderer_DrawLine2f(G_Renderer, 
                            BonkLine,
                            Mode->PlayerCircleRadius * 2,
                            C4f_Green,
                            ZOrder);
        
        circle2f StartCircle = Circle2f_Create(BonkLine.Min, 
                                               Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(G_Renderer,
                              StartCircle,
                              1.f, 
                              8, 
                              C4f_Green, 
                              ZOrder);
        
        
        circle2f EndCircle = Circle2f_Create(BonkLine.Max, 
                                             Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(G_Renderer,
                              EndCircle,
                              1.f, 
                              8, 
                              C4f_Green, 
                              ZOrder);
    }
    
}

#endif 
