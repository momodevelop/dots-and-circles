#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items


static inline void
Draw_TexturedQuadFromImage(image_id ImageId,
                           m44f Transform,
                           c4f Color = C4F_WHITE) 
{
    image* Image = Assets_GetImage(G_Assets, ImageId);
    texture* Texture = Assets_GetTexture(G_Assets, Image->TextureId);
    quad2f AtlasUV = Assets_GetAtlasUV(G_Assets, Image);
    
    Renderer_DrawTexturedQuad(G_Renderer,
                              Color,
                              Transform,
                              Texture->Handle,
                              AtlasUV);
}

static inline void
Draw_Text(font_id FontId,
          v3f Position,
          String str,
          f32 Size, 
          c4f Color = C4F_WHITE) 
{
    v3f CurPosition = Position;
    font* Font = Assets_GetFont(G_Assets, FontId);
    
    f32 ZLayerOffset = 0.f;
    for(u32 I = 0; I < str.count; ++I) {
        font_glyph* Glyph = Font_GetGlyph(Font, str.data[I]);
        aabb2f Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = m44f::create_translation(0.5f, 0.5f, 0.f); 
        
        f32 BoxWidth = width(Box);
        f32 BoxHeight = height(Box);
        
        m44f S = m44f::create_scale(BoxWidth * Size, 
                                    BoxHeight* Size, 
                                    1.f);
        
        m44f T = m44f::create_translation(CurPosition.x + Box.min.x * Size, 
                                          CurPosition.y + Box.min.y * Size,  
                                          CurPosition.z + ZLayerOffset);
        
        
        Draw_TexturedQuadFromImage(Glyph->ImageId,
                                   T*S*A,
                                   Color);
        
        CurPosition.x += Glyph->Advance * Size;
        if (I != str.count - 1 ) {
            u32 Kerning = Font_GetKerning(Font, 
                                          str.data[I], 
                                          str.data[I+1]);
            CurPosition.x += Kerning * Size;
            
        }
        ZLayerOffset += 0.001f;
        
    }
}

#endif

