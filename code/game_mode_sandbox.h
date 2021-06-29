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
        Mode->Camera.Position = v3f::create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = v3f::create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = C4F_GREY2;
        Mode->Camera.Dimensions = v3f::create(Game_DesignWidth,
                                              Game_DesignHeight,
                                              Game_DesignDepth);
    }
    
    
    const f32 BulletRadius = 8.f;
    f32 StartX = Game_DesignWidth * -0.5f + BulletRadius;
    f32 StartY = Game_DesignHeight * -0.5f + BulletRadius;
    f32 OffsetX = 0.f;
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < ARRAY_COUNT(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        B->Position = v2f::create(StartX + OffsetX, StartY + OffsetY);
        B->HitCircle = circle2f::create({}, BulletRadius);
        
        OffsetX += BulletRadius * 2; // padding
        if(OffsetX >= Game_DesignWidth) {
            OffsetX = 0.f;
            OffsetY += BulletRadius * 2;
        }
        B->Velocity = v2f::create(0.f, 0.f);
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
    for (u32 I = 0; I < ARRAY_COUNT(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        
        B->Position += B->Velocity * DeltaTime;
        
    }
    
    
    // NOTE(Momo): Line to circle collision
    line2f BonkLine = line2f::create(Mode->PrevMousePos, Mode->CurMousePos);
    if (Mode->ClickCount >= 2) {
        if (!is_equal(Mode->PrevMousePos, Mode->CurMousePos)) {
            
            for (u32 I = 0; I < ARRAY_COUNT(Mode->Bullets); ++I) {
                game_mode_sandbox_bullet* B = Mode->Bullets + I;
                if (B->IsHit) {
                    continue;
                }
                circle2f C = B->HitCircle;
                C.origin = B->Position;
                
                circle2f PC = circle2f::create(Mode->PrevMousePos,
                                               Mode->PlayerCircleRadius);
                v2f PCVel = Mode->CurMousePos - Mode->PrevMousePos;
                v2f TimedVelocity = B->Velocity * DeltaTime;
                
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
    for (u32 I = 0; I < ARRAY_COUNT(Mode->Bullets); ++I) {
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        if (B->IsHit) {
            continue;
        }
        m44f S = m44f::create_scale(B->HitCircle.radius*2, B->HitCircle.radius*2, 1.f);
        m44f T = m44f::create_translation(B->Position.x,
                                  B->Position.y,
                                  ZOrder += 0.001f);
        
        Draw_TexturedQuadFromImage(Image_BulletDot,
                                   T*S, 
                                   c4f{1.f, 1.f, 1.f, 0.5f});
    }
    
    // NOTE(Momo) Render Lines
    if (Mode->ClickCount >= 2) {
        ZOrder = 10.f;
        Renderer_DrawLine2f(G_Renderer, 
                            BonkLine,
                            Mode->PlayerCircleRadius * 2,
                            C4F_GREEN,
                            ZOrder);
        
        circle2f StartCircle = circle2f::create(BonkLine.min, 
                                                Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(G_Renderer,
                              StartCircle,
                              1.f, 
                              8, 
                              C4F_GREEN, 
                              ZOrder);
        
        
        circle2f EndCircle = circle2f::create(BonkLine.max, 
                                              Mode->PlayerCircleRadius);
        Renderer_DrawCircle2f(G_Renderer,
                              EndCircle,
                              1.f, 
                              8, 
                              C4F_GREEN, 
                              ZOrder);
    }
    
}

#endif 
