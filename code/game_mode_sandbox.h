#ifndef GAME_MODE_ATLAS_TEST_H
#define GAME_MODE_ATLAS_TEST_H

#include "game.h"

// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_entity {
    v3f Position;
};

struct game_mode_sandbox {
    game_mode_sandbox_entity Entity;
};

static inline void 
InitSandboxMode(game_state* GameState) {
    game_mode_sandbox* Mode = GameState->SandboxMode;     
    Mode->Entity.Position = v3f{ -800.f, 0.f };
}


static inline void
DrawString(mailbox* RenderCommands, 
           game_assets* Assets,
           v3f Position, f32 Size, const char* String) 
{
    v3f CurPosition = Position;
    
    
    v4f Color = { 1.f, 1.f, 1.f, 1.f };
    auto* Font = Assets->Fonts + Font_Default;
    
    u32 Strlen = SiStrLen(String);
    for(u32 i = 0; i < Strlen; ++i) {
        auto* Glyph = Font->Glyphs + HashCodepoint(String[i]);
        auto Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = M44fTranslation(0.5f, 0.5f, 0.f); 
        m44f S = M44fScale(Width(Box) * Size, 
                             Height(Box) * Size, 
                             1.f);
        
        m44f T = M44fTranslation(CurPosition.X + Box.Min.X * Size, 
                                   CurPosition.Y + Box.Min.Y * Size,  
                                   CurPosition.Z);
        
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    T*S*A, 
                                    Glyph->BitmapId,
                                    GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += Glyph->Advance * Size;
        if (String[i+1] != 0 ) {
            CurPosition.X += Font->Kernings[String[i]][String[i+1]] * Size;
        }
    }
    
    
    
}

static inline void
UpdateSandboxMode(game_state* GameState, 
       mailbox* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
    game_mode_sandbox* Mode = GameState->SandboxMode;

    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushCommandOrthoCamera(RenderCommands, 
            v3f{}, 
            CenteredRect( 
                v3f{ DesignWidth, DesignHeight, DesignDepth }, 
                v3f{ 0.5f, 0.5f, 0.5f }
            )
    );
    
    // NOTE(Momo): Image Test
    {
        game_mode_sandbox_entity * Entity = &Mode->Entity;
        m44f Transform = M44fTranslation(Mode->Entity.Position) * M44fScale(64.f, 64.f, 1.f);
        auto* AtlasRect = GameState->Assets.AtlasRects + AtlasRect_PlayerDot;
       
        v3f Speed = { 50.f, 0.f, 0.f };
        Entity->Position += Speed * DeltaTime;

        PushCommandDrawTexturedQuad(RenderCommands, 
                                    ColorWhite, 
                                    Transform, 
                                    AtlasRect->BitmapId,
                                    GetAtlasUV(&GameState->Assets, AtlasRect));
    }



#if 0
    // NOTE(Momo): Font test
    {
        DrawString(RenderCommands, 
                   &GameState->Assets, 
                   //{ -250.f, -320.f, 0.f }, 
                   {},
                   72.f, 
                   //"Hello! I'm Ryoji!");
                   "The quick brown fox jump");
    }
#endif 
    
    
}



#endif 