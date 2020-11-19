#ifndef __GAME_TEXT__
#define __GAME_TEXT__

#include "mm_string.h"

#include "game_renderer.h"
#include "game_assets.h"

static inline void
DrawText(mmcmd_commands* RenderCommands, 
           game_assets* Assets,
           mmm_v3f Position,
           mmm_v4f Color, 
           font_id FontId,
           f32 Size, 
           mms_string String) 
{
    mmm_v3f CurPosition = Position;
    
    for(u32 i = 0; i < String.Length; ++i) {
        font* Font = Assets->Fonts + FontId;
        auto* Glyph = Font->Glyphs + HashCodepoint(String[i]);
        auto Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        mmm_m44f O = mmm_Translation(0.5f, 0.5f, 0.f); 
        mmm_m44f S = mmm_Scale(mmm_Width(Box) * Size, 
                             mmm_Height(Box) * Size, 
                             1.f);
        
        mmm_m44f T = mmm_Translation(CurPosition.X + Box.Min.X * Size, 
                                   CurPosition.Y + Box.Min.Y * Size,  
                                   CurPosition.Z);
        
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    T*S*O,
                                    Glyph->BitmapId,
                                    GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += Glyph->Advance * Size;
        if (i != String.Length - 1 ) {
            CurPosition.X += Font->Kernings[HashCodepoint(String[i])][HashCodepoint(String[i+1])] * Size;
        }
    }
    
    
    
}

#endif

