#ifndef GAME_MODE_ATLAS_TEST_H
#define GAME_MODE_ATLAS_TEST_H

#include "game.h"


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_atlas_test {
    static constexpr u8 TypeId = 3;
};

static inline void 
Init(game_mode_atlas_test* Mode, game_state* GameState) {
    Log("Atlas Test state initialized!");
}

static inline void
DrawString(commands* RenderCommands, 
           game_assets* Assets,
           v3f Position, f32 Scale, const char* String) 
{
    v3f CurPosition = Position;
    
    
    v4f Color = { 1.f, 1.f, 1.f, 1.f };
    auto* Font = Assets->Fonts + Font_Default;
    u32 Strlen = NtsLength(String);
    for(u32 i = 0; i < Strlen; ++i) {
        auto Glyph = Font->Glyphs[GetGlyphIndexFromCodepoint(String[i])];
        auto Rect = Glyph.Rect;
        
        m44f S = ScaleMatrix((f32)GetWidth(Rect), 
                             (f32)GetHeight(Rect), 
                             1.f);
        
        m44f T = TranslationMatrix(CurPosition.X + (f32)GetWidth(Rect)*0.5f, 
                                   CurPosition.Y + (f32)GetHeight(Rect)*0.5f, 
                                   CurPosition.Z);
        m44f Transform = T*S;
        
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    Glyph.BitmapId,
                                    GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += GetWidth(Rect);
    }
    
    
    
}

static inline void
Update(game_mode_atlas_test* Mode,
       game_state* GameState, 
       commands* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
    
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushCommandSetOrthoBasis(RenderCommands, { 0.f, 0.f, 0.f }, { 1600.f, 900.f, 200.f });
    
    // NOTE(Momo): Image Test
    {
        v4f Color = { 1.f, 1.f, 1.f, 1.f };
        m44f Transform = ScaleMatrix(500.f, 500.f, 1.f);
        auto AtlasRect = GameState->Assets->AtlasRects[AtlasRect_Ryoji];
        
        auto TextureHandle = AtlasRect.BitmapId;
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    AtlasRect.BitmapId,
                                    GetAtlasUV(GameState->Assets, AtlasRect));
    }
    
    // NOTE(Momo): Font test
    {
        DrawString(RenderCommands, 
                   GameState->Assets, 
                   { -150.f, -320.f, 0.f }, 
                   24.f, 
                   "Hello! I'm Ryoji!");
    }
    
    
    
}



#endif 
