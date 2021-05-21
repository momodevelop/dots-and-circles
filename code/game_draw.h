#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items


static inline void
Draw_TexturedQuadFromImage(mailbox* RenderCommands,
                           assets* Assets,
                           image_id ImageId,
                           MM_M44f Transform,
                           c4f Color = Color_White) 
{
    image* Image = Assets_GetImage(Assets, ImageId);
    texture* Texture = Assets_GetTexture(Assets, Image->TextureId);
    MM_Quad2f AtlasUV = Assets_GetAtlasUV(Assets, Image);
    
    Renderer_DrawTexturedQuad(RenderCommands,
                              Color,
                              Transform,
                              Texture->Handle,
                              AtlasUV);
}



static inline void
Draw_Text(mailbox* RenderCommands, 
          assets* Assets,
          font_id FontId,
          MM_V3f Position,
          u8_cstr String,
          f32 Size, 
          c4f Color = Color_White) 
{
    MM_V3f CurPosition = Position;
    font* Font = Assets_GetFont(Assets, FontId);
    
    f32 ZLayerOffset = 0.f;
    for(u32 I = 0; I < String.count; ++I) {
        font_glyph* Glyph = Font_GetGlyph(Font, String[I]);
        MM_Aabb2f Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        MM_M44f A = MM_M44f_Translation(0.5f, 0.5f, 0.f); 
        
        f32 BoxWidth = MM_Aabb2f_Width(Box);
        f32 BoxHeight = MM_Aabb2f_Height(Box);
        
        MM_M44f S = MM_M44f_Scale(BoxWidth * Size, 
                            BoxHeight* Size, 
                            1.f);
        
        MM_M44f T = MM_M44f_Translation(CurPosition.x + Box.min.x * Size, 
                                  CurPosition.y + Box.min.y * Size,  
                                  CurPosition.z + ZLayerOffset);
        
        MM_M44f SA = MM_M44f_Concat(S,A);
        MM_M44f TSA = MM_M44f_Concat(T, SA);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Glyph->ImageId,
                                   TSA,
                                   Color);
        
        CurPosition.x += Glyph->Advance * Size;
        if (I != String.count - 1 ) {
            u32 Kerning = Font_GetKerning(Font, 
                                          String[I], 
                                          String[I+1]);
            CurPosition.x += Kerning * Size;
            
        }
        ZLayerOffset += 0.001f;
        
    }
}

#endif

