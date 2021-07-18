#include <stdio.h>
#include <stdlib.h>
#include "momo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "game_config.h"
#include "game_assets_file.h"  

#include "tool_build_assets.h"
#include "tool_build_assets_util.h"
#include "tool_build_assets_atlas.h"
#include "tool_build_assets_data.h"


#define GENERATE_TEST_PNG 1
#define ASSET_BUILDER_MEMORY_SIZE MEBIBYTES(8)
#define ASSET_BUILDER_MEM_CHECK printf("[Memcheck] Line %d: %d bytes used\n", __LINE__, arena.used)


int main() {
    printf("Start!\n");
    
    void* memory = malloc(ASSET_BUILDER_MEMORY_SIZE);
    if (!memory) {
        printf("Failed to initialize memory");
        return 1; 
    }
    defer { free(memory); };
    
    Arena arena = {};
    Arena_Init(&arena, memory, ASSET_BUILDER_MEMORY_SIZE);
    
    //~ NOTE(Momo): Load font
    
    // TODO(Momo): Preload stuff related to font that we actually need and then free
    // loaded font's data to be more memory efficient.
    Loaded_Font loaded_font = {};
    if (!load_font(&loaded_font, &arena, "assets/DroidSansMono.ttf")) 
    {
        printf("[Atlas] Failed to load font\n");
        return 1; 
    }
    f32 font_pixel_scale = stbtt_ScaleForPixelHeight(&loaded_font.info, 1.f); 
    
    //~ NOTE(Momo): Initialize context for images in atlas
    
    
    Atlas_Context_Image atlas_image_contexts[ARRAY_COUNT(g_image_contexts)] = {};
    for (u32 I = 0; I < ARRAY_COUNT(g_image_contexts); ++I) {
        Asset_Builder_Image_Context* builder_context = g_image_contexts + I;
        Atlas_Context_Image* atlas_context = atlas_image_contexts + I;
        
        atlas_context->type = ATLAS_CONTEXT_TYPE_IMAGE;
        atlas_context->filename = builder_context->filename;
        atlas_context->id = builder_context->image_id;
        atlas_context->texture_id = builder_context->texture_id;
    }
    
    
    
    //~ NOTE(Momo): Initialize contexts for font images in atlas
    Atlas_Context_Font atlas_font_contexts[FONT_GLYPH_COUNT];
    
    const u32 aabb_count = ARRAY_COUNT(atlas_image_contexts) + ARRAY_COUNT(atlas_font_contexts);
    aabb2u aabbs[aabb_count] = {};
    void* user_datas[aabb_count] = {};
    u32 aabb_counter = 0;
    for (u32 I = 0; I < ARRAY_COUNT(atlas_image_contexts); ++I ) {
        Atlas_Context_Image* Context = atlas_image_contexts + I;
        aabb2u* aabb = aabbs + aabb_counter;
        
        s32 W, H, C;
        stbi_info(Context->filename, &W, &H, &C);
        
        (*aabb) = aabb2u::create_wh((u32)W, (u32)H);
        
        
        user_datas[aabb_counter] = Context;
        
        ++aabb_counter;
    }
    
    for (u32 i = 0; i < ARRAY_COUNT(atlas_font_contexts); ++i) {
        Atlas_Context_Font* font = atlas_font_contexts + i;
        font->type = ATLAS_CONTEXT_TYPE_FONT;
        font->loaded_font = &loaded_font;
        font->codepoint = FONT_GLYPH_CODEPOINT_START + i;
        font->raster_scale = stbtt_ScaleForPixelHeight(&loaded_font.info, 72.f);
        font->font_id = FONT_DEFAULT;
        font->image_id = Image_ID(IMAGE_FONT_START + i);
        font->texture_id = TEXTURE_ATLAS_DEFAULT;
        
        aabb2u* aabb = aabbs + aabb_counter;
        s32 ix0, iy0, ix1, iy1;
        stbtt_GetCodepointTextureBox(&font->loaded_font->info, 
                                     font->codepoint, 
                                     font->raster_scale, 
                                     font->raster_scale, 
                                     &ix0, &iy0, &ix1, &iy1);
        
        (*aabb) = aabb2u::create_wh((u32)(ix1 - ix0), (u32)(iy1 - iy0));
        user_datas[aabb_counter] = font;
        
        ++aabb_counter;
    }
    
    
    //~ NOTE(Momo): Pack rects
    u32 atlas_width = 1024;
    u32 atlas_height = 1024;
    if (!pack_aabbs(&arena,
                    atlas_width,
                    atlas_height,
                    aabbs,
                    aabb_count, 
                    AABB_PACKER_SORT_HEIGHT)) 
    {
        printf("[Atlas] Failed to generate texture\n");
        return 1;
    }
    
    // NOTE(Momo): Generate atlas from rects
    u8* atlas_texture = generate_atlas(&arena,
                                       aabbs, 
                                       user_datas,
                                       aabb_count, 
                                       atlas_width, 
                                       atlas_height);
    if (!atlas_texture) {
        printf("[Atlas] Cannot generate atlas, not enough memory\n");
        return 1;
    }
#if GENERATE_TEST_PNG 
    stbi_write_png("test.png", atlas_width, atlas_height, 4, atlas_texture, atlas_width*4);
    printf("[Atlas] Written test atlas: test.png\n");
#endif
    printf("[Atlas] Atlas generated\n");
    
    //~ NOTE(Momo): Actual asset building
    printf("[Build Assets] Building Assets\n");
    
    Asset_Builder asset_builder = {};
    
    asset_builder.begin(GAME_ASSET_FILENAME,
                        GAME_ASSET_FILE_SIGNATURE);
    
    // NOTE(Momo): Write atlas
    {
        asset_builder.write_texture(TEXTURE_ATLAS_DEFAULT, 
                                    atlas_width, 
                                    atlas_height, 
                                    4, 
                                    atlas_texture);
        
        // NOTE(Momo): Write atlas aabb for images in font
        for(u32 i = 0; i <  aabb_count; ++i) {
            aabb2u aabb = aabbs[i];
            auto type = *(Atlas_Context_Type*)user_datas[i];
            switch(type) {
                case ATLAS_CONTEXT_TYPE_IMAGE: {
                    auto* image = (Atlas_Context_Image*)user_datas[i];
                    asset_builder.write_image(image->id, 
                                              image->texture_id, 
                                              aabb);
                } break;
                case ATLAS_CONTEXT_TYPE_FONT: {
                    auto* font  = (Atlas_Context_Font*)user_datas[i];
                    
                    s32 advance;
                    s32 left_side_bearing; 
                    stbtt_GetCodepointHMetrics(&loaded_font.info, 
                                               font->codepoint, 
                                               &advance, 
                                               &left_side_bearing);
                    
                    aabb2i box;
                    stbtt_GetCodepointBox(&loaded_font.info, 
                                          font->codepoint, 
                                          &box.min.x, 
                                          &box.min.y, 
                                          &box.max.x, 
                                          &box.max.y);
                    
                    aabb2f scaled_box = mul(to_aabb2f(box), font_pixel_scale);
                    asset_builder.write_font_glyph(font->font_id, 
                                                   font->image_id,
                                                   font->texture_id,
                                                   font->codepoint, font_pixel_scale * advance,
                                                   font_pixel_scale * left_side_bearing,
                                                   aabb,
                                                   scaled_box);
                    
                } break;
                
            }
        }
    }
    
    // NOTE(Momo): font info
    {
        s32 ascent, descent, line_gap;
        stbtt_GetFontVMetrics(&loaded_font.info, &ascent, &descent, &line_gap); 
        
        aabb2i bounding_box = {}; 
        stbtt_GetFontBoundingBox(&loaded_font.info, 
                                 &bounding_box.min.x,
                                 &bounding_box.min.y,
                                 &bounding_box.max.x,
                                 &bounding_box.max.y
                                 );
        printf("[Build Assets] Writing font information...\n");
        asset_builder.write_font(FONT_DEFAULT, 
                                 ascent * font_pixel_scale, 
                                 descent * font_pixel_scale, 
                                 line_gap * font_pixel_scale); 
    }
    
    // NOTE(Momo): Kerning info
    {
        printf("[Build Assets] Writing kerning...\n");
        for (u32 i = FONT_GLYPH_CODEPOINT_START;
             i <= FONT_GLYPH_CODEPOINT_END; 
             ++i) 
        {
            for(u32 j = FONT_GLYPH_CODEPOINT_START;
                j <= FONT_GLYPH_CODEPOINT_END; 
                ++j) 
            {
                s32 kerning = stbtt_GetCodepointKernAdvance(&loaded_font.info, (s32)i, (s32)j);
                asset_builder.write_font_kerning(FONT_DEFAULT, i, j, kerning);
            }
        }
    }
    
    
    //NOTE(Momo): Anime
    printf("[Build Assets] Writing Anime...\n");
    {
        for(u32 i = 0; i < ARRAY_COUNT(g_anime_contexts); ++i) {
            Asset_Builder_Anime_Context * ctx = g_anime_contexts + i;
            asset_builder.write_anime(ctx->id,
                                      ctx->frames,
                                      ctx->frame_count);
        }
    }
    
    // NOTE(Momo): Msg
    printf("[Build Assets] Writing Msg...\n");
    {
        for(u32 i = 0; i < ARRAY_COUNT(g_msg_contexts); ++i) {
            Asset_Builder_Msg_Context* ctx = g_msg_contexts + i;
            asset_builder.write_msg(ctx->id,
                                    ctx->str);
        }
    }
    
    
    printf("[Build Assets] Writing Sound...\n");
    {
        for(u32 I = 0; I < ARRAY_COUNT(g_sound_contexts); ++I) {
            Asset_Builder_Sound_Context* ctx = g_sound_contexts + I;
            
            Arena_Marker mark = Arena_Mark(&arena);
            defer { Arena_Revert(&mark); };
            
            Read_File_Result file_result = {};
            if(!read_file_into_memory(&file_result, mark, ctx->filename)) {
                return 1;
            }
            
            Wav_Load_Result wav_result = {};
            if(!load_wav_from_memory(&wav_result,
                                     file_result.data,
                                     file_result.size)) {
                return 1;
            }
            asset_builder.write_sound(ctx->id,
                                      &wav_result);
        }
    }
    asset_builder.end();
    printf("[Build Assets] Assets Built\n");
    
    ASSET_BUILDER_MEM_CHECK;
    return 0;
    
}

