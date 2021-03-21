#ifndef __GAME_DRAW__
#define __GAME_DRAW__


#include "game_renderer.h"
#include "game_assets.h"


static inline void
DrawText(mailbox* RenderCommands, 
         game_assets* Assets,
         game_asset_font_id FontId,
         v3f Position,
         u8_cstr String,
         f32 Size, 
         c4f Color = Color_White) 
{
    v3f CurPosition = Position;
    auto* Font = GetFont(Assets, FontId);
    
    for(u32 I = 0; I < String.Size; ++I) {
        game_asset_font_glyph* Glyph = Font->Glyphs + HashCodepoint(String.Data[I]);
        aabb2f Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = M44f_Translation(0.5f, 0.5f, 0.f); 
        
        f32 BoxWidth = Aabb2f_Width(Box);
        f32 BoxHeight = Aabb2f_Height(Box);
        
        m44f S = M44f_Scale(BoxWidth * Size, 
                            BoxHeight* Size, 
                            1.f);
        
        m44f T = M44f_Translation(CurPosition.X + Box.Min.X * Size, 
                                  CurPosition.Y + Box.Min.Y * Size,  
                                  CurPosition.Z);
        
        m44f SA = M44f_Concat(S,A);
        m44f TSA = M44f_Concat(T, SA);
        
        PushDrawTexturedQuad(RenderCommands, 
                             Color, 
                             TSA,
                             GetTexture(Assets, Glyph->TextureId)->Handle,
                             GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += Glyph->Advance * Size;
        if (I != String.Size - 1 ) {
            CurPosition.X += 
                Font->Kernings[HashCodepoint(String.Data[I])][HashCodepoint(String.Data[I+1])] * Size;
        }
    }
    
    
    
}

#endif

