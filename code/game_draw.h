#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items


static inline void
draw_textured_quad_from_image(Image_ID image_id,
                              m44f transform,
                              c4f color = C4F_WHITE) 
{
    Image* image = G_Assets->get_image(image_id);
    Texture* texture = G_Assets->get_texture(image->texture_id);
    quad2f atlas_uv = G_Assets->get_atlas_uv(image);
    
    Renderer_DrawTexturedQuad(G_Renderer,
                              color,
                              transform,
                              texture->handle,
                              atlas_uv);
}

static inline void
draw_text(Font_ID font_id,
          v3f position,
          String str,
          f32 size, 
          c4f color = C4F_WHITE) 
{
    v3f cur_position = position;
    Font* font = G_Assets->get_font(font_id);
    
    f32 z_layer_offset = 0.f;
    for(u32 i = 0; i < str.count; ++i) {
        Font_Glyph* glyph = font->get_glyph(str.data[i]);
        aabb2f box = glyph->box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = m44f::create_translation(0.5f, 0.5f, 0.f); 
        
        f32 boxWidth = width(box);
        f32 boxHeight = height(box);
        
        m44f S = m44f::create_scale(boxWidth * size, 
                                    boxHeight* size, 
                                    1.f);
        
        m44f T = m44f::create_translation(cur_position.x + box.min.x * size, 
                                          cur_position.y + box.min.y * size,  
                                          cur_position.z + z_layer_offset);
        
        
        draw_textured_quad_from_image(glyph->image_id,
                                      T*S*A,
                                      color);
        
        cur_position.x += glyph->advance * size;
        if (i != str.count - 1 ) {
            u32 kerning = font->get_kerning(str.data[i], 
                                            str.data[i+1]);
            cur_position.x += kerning * size;
        }
        z_layer_offset += 0.001f;
        
    }
}

#endif

