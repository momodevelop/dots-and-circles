#ifndef __GAME_DRAW__
#define __GAME_DRAW__

#include "mm_array.h"

#include "game_renderer.h"
#include "game_assets.h"


static inline void
DrawText(mailbox* RenderCommands, 
         game_assets* Assets,
         game_asset_font_id FontId,
         v3f Position,
         string String,
         f32 Size, 
         c4f Color = Color_White) 
{
    v3f CurPosition = Position;
    auto* Font = GetFont(Assets, FontId);
    
    for(u32 i = 0; i < String.Count; ++i) {
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

        PushDrawTexturedQuad(RenderCommands, 
                             Color, 
                             T*S*A,
                             GetTexture(Assets, Glyph->TextureId)->Handle,
                             GetAtlasUV(Assets, Glyph));
 
        CurPosition.X += Glyph->Advance * Size;
        if (i != String.Count - 1 ) {
            CurPosition.X += 
                Font->Kernings[HashCodepoint(String[i])][HashCodepoint(String[i+1])] * Size;
        }
    }
    
    
    
}

#endif

