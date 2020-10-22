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
DrawString(mmcmd_commands* RenderCommands, 
           game_assets* Assets,
           mmm_v3f Position, f32 Size, const char* String) 
{
    mmm_v3f CurPosition = Position;
    
    
    mmm_v4f Color = { 1.f, 1.f, 1.f, 1.f };
    auto* Font = Assets->Fonts + Font_Default;
    
    u32 Strlen = StrLen(String);
    for(u32 i = 0; i < Strlen; ++i) {
        auto* Glyph = Font->Glyphs + GetGlyphIndexFromCodepoint(String[i]);
        auto Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        mmm_m44f BLT = mmm_Translation(0.5f, 0.5f, 0.f); 
        mmm_m44f S = mmm_Scale(mmm_Width(Box) * Size, 
                             mmm_Height(Box) * Size, 
                             1.f);
        
        mmm_m44f T = mmm_Translation(CurPosition.X + Box.Min.X * Size, 
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
       mmcmd_commands* RenderCommands, 
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
        mmm_v4f Color = { 1.f, 1.f, 1.f, 1.f };
        mmm_m44f Transform = mmm_Scale(5.f, 5.f, 1.f);
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
