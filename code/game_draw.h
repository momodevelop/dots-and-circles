#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items


static inline void
DrawTexturedQuadFromImage(Image_ID image_id,
                          m44f transform,
                          c4f color = C4F_WHITE) 
{
    Image* image = Assets_GetImage(g_assets, image_id);
    Texture* texture = Assets_GetTexture(g_assets, image->texture_id);
    quad2f atlas_uv = Assets_GetAtlasUV(g_assets, image);
    
    Renderer_DrawTexturedQuad(g_renderer,
                              color,
                              transform,
                              texture->handle,
                              atlas_uv);
}

static inline void
DrawText(Font_ID font_id,
         v3f position,
         String str,
         f32 size, 
         c4f color = C4F_WHITE) 
{
    v3f cur_position = position;
    Font* font = Assets_GetFont(g_assets, font_id);
    
    f32 z_layer_offset = 0.f;
    for(u32 i = 0; i < str.count; ++i) {
        Font_Glyph* glyph = Font_GetGlyph(font, str.data[i]);
        aabb2f box = glyph->box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = m44f_Translation(0.5f, 0.5f, 0.f); 
        
        f32 boxWidth = Width(box);
        f32 boxHeight = Height(box);
        
        m44f S = m44f_Scale(boxWidth * size, 
                            boxHeight* size, 
                            1.f);
        
        m44f T = m44f_Translation(cur_position.x + box.min.x * size, 
                                  cur_position.y + box.min.y * size,  
                                  cur_position.z + z_layer_offset);
        
        
        DrawTexturedQuadFromImage(glyph->image_id,
                                  T*S*A,
                                  color);
        
        cur_position.x += glyph->advance * size;
        if (i != str.count - 1 ) {
            u32 kerning = Font_GetKerning(font, str.data[i], str.data[i+1]);
            cur_position.x += kerning * size;
        }
        z_layer_offset += 0.001f;
        
    }
}

#endif

