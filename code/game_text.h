#ifndef __GAME_TEXT__
#define __GAME_TEXT__

#include "mm_array.h"

#include "game_renderer.h"
#include "game_assets.h"

static inline void
DrawText(mailbox* RenderCommands, 
           game_assets* Assets,
           v3f Position,
           v4f Color, 
           font_id FontId,
           f32 Size, 
           string String) 
{
    v3f CurPosition = Position;
    
    for(u32 i = 0; i < String.Length; ++i) {
        font* Font = Assets->Fonts + FontId;
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
        if (i != String.Length - 1 ) {
            CurPosition.X += Font->Kernings[HashCodepoint(String[i])][HashCodepoint(String[i+1])] * Size;
        }
    }
    
    
    
}

#endif

