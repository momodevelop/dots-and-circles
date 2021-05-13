#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items


static inline void
Draw_TexturedQuadFromImage(mailbox* RenderCommands,
                           assets* Assets,
                           image_id ImageId,
                           m44f Transform,
                           c4f Color = Color_White) 
{
    image* Image = Assets_GetImage(Assets, ImageId);
    texture* Texture = Assets_GetTexture(Assets, Image->TextureId);
    quad2f AtlasUV = Assets_GetAtlasUV(Assets, Image);
    
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
          v3f Position,
          u8_cstr String,
          f32 Size, 
          c4f Color = Color_White) 
{
    v3f CurPosition = Position;
    font* Font = Assets_GetFont(Assets, FontId);
    
    f32 ZLayerOffset = 0.f;
    for(u32 I = 0; I < String.Count; ++I) {
        font_glyph* Glyph = Font_GetGlyph(Font, String.Data[I]);
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
                                  CurPosition.Z + ZLayerOffset);
        
        m44f SA = M44f_Concat(S,A);
        m44f TSA = M44f_Concat(T, SA);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Glyph->ImageId,
                                   TSA,
                                   Color);
        
        CurPosition.X += Glyph->Advance * Size;
        if (I != String.Count - 1 ) {
            u32 Kerning = Font_GetKerning(Font, 
                                          String.Data[I], 
                                          String.Data[I+1]);
            CurPosition.X += Kerning * Size;
            
        }
        ZLayerOffset += 0.001f;
        
    }
}

#endif

