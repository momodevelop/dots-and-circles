#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_bullet {
    v2f Position;
    circle2f HitCircle;
};
struct game_mode_sandbox {
    game_mode_sandbox_bullet Bullets[2500];
    game_camera Camera;
    
    v2f PrevMousePos;
    b32 FirstTime;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
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
    }
    
    Mode->FirstTime = False;
    
}




static inline void
UpdateSandboxMode(permanent_state* PermState, 
                  transient_state* TranState,
                  mailbox* RenderCommands, 
                  game_input* Input,
                  f32 DeltaTime) 
{
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    assets* Assets = &TranState->Assets;
    
    // Update
    if (Button_IsPoked(Input->ButtonSwitch)) {
        if(Mode->FirstTime) {
            Mode->PrevMousePos = Input->;
        }
    }
    
    // Render
    Camera_Set(&Mode->Camera, RenderCommands);
    f32 ZOrder = 0.f;
    for (u32 I = 0; I < ArrayCount(Mode->Bullets); ++I) {
        
        game_mode_sandbox_bullet* B = Mode->Bullets + I;
        m44f S = M44f_Scale(B->HitCircle.Radius*2, B->HitCircle.Radius*2, 1.f);
        m44f T = M44f_Translation(B->Position.X,
                                  B->Position.Y,
                                  ZOrder += 0.001f);
        
        Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                       Assets,
                                       AtlasAabb_BulletDot,
                                       M44f_Concat(T,S), 
                                       Color_White);
    }
}

#endif 
