#ifndef GAME_MODE_ATLAS_TEST_H
#define GAME_MODE_ATLAS_TEST_H

// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_atlas_test {
};

static inline void 
Init(game_mode_atlas_test* Mode, game_state* GameState) {
    Log("Atlas Test state initialized!");
}

static inline void
DrawString(commands* RenderCommands, 
           game_assets* Assets,
           v3f Position, f32 Size, const char* String) 
{
    v3f CurPosition = Position;
    
    
    v4f Color = { 1.f, 1.f, 1.f, 1.f };
    auto* Font = Assets->Fonts + Font_Default;
    
    u32 Strlen = NtsLength(String);
    for(u32 i = 0; i < Strlen; ++i) {
        auto* Glyph = Font->Glyphs + GetGlyphIndexFromCodepoint(String[i]);
        auto Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f BLT = TranslationMatrix(0.5f, 0.5f, 0.f); 
        m44f S = ScaleMatrix(GetWidth(Box) * Size, 
                             GetHeight(Box) * Size, 
                             1.f);
        
        m44f T = TranslationMatrix(CurPosition.X + Box.Min.X * Size, 
                                   CurPosition.Y + Box.Min.Y * Size,  
                                   CurPosition.Z);
        
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    T*S*BLT, 
                                    Glyph->BitmapId,
                                    GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += Glyph->Advance * Size;
        if (String[i+1] != 0 ) {
            CurPosition.X += Font->Kernings[String[i]][String[i+1]] * Size;
        }
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
    
    
#if 1
    // NOTE(Momo): Image Test
    {
        v4f Color = { 1.f, 1.f, 1.f, 1.f };
        m44f Transform = ScaleMatrix(5.f, 5.f, 1.f);
        auto* AtlasRect = GameState->Assets->AtlasRects + AtlasRect_Ryoji;
        
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    AtlasRect->BitmapId,
                                    GetAtlasUV(GameState->Assets, AtlasRect));
    }
#endif
    // NOTE(Momo): Font test
    {
        DrawString(RenderCommands, 
                   GameState->Assets, 
                   //{ -250.f, -320.f, 0.f }, 
                   {},
                   72.f, 
                   //"Hello! I'm Ryoji!");
                   "The quick brown fox jump");
    }
    
    
    
}



#endif 
