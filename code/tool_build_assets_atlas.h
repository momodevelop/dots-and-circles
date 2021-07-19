/* date = March 14th 2021 2:41 pm */

#ifndef TOOL_BUILD_ASSETS_ATLAS_H
#define TOOL_BUILD_ASSETS_ATLAS_H


// NOTE(Momo):  Atlas stuff ////////////////////////
enum Atlas_Context_Type {
    ATLAS_CONTEXT_TYPE_IMAGE,
    ATLAS_CONTEXT_TYPE_FONT,
};

struct Atlas_Context_Image {
    Atlas_Context_Type type;
    const char* filename;
    Image_ID id;
    Texture_ID texture_id;
    u8* texture;
};


struct Atlas_Context_Font {
    Atlas_Context_Type type;
    Font_ID font_id;
    Texture_ID texture_id;
    Image_ID image_id;
    f32 raster_scale;
    u32 codepoint;
    Loaded_Font* loaded_font;
    u8* texture;
};    


static inline void 
write_sub_texture_to_atlas(u8** atlas_memory, u32 atlas_width, u32 atlas_height,
                           u8* texture_memory, aabb2u texture_aabb) 
{
    s32 j = 0;
    u32 texture_aabb_w = Width(texture_aabb);
    u32 texture_aabb_h = Height(texture_aabb);
    
    for (u32 y = texture_aabb.min.y; y < texture_aabb.min.y + texture_aabb_h; ++y) {
        for (u32 x = texture_aabb.min.x; x < texture_aabb.min.x + texture_aabb_w; ++x) {
            u32 Index = (x + y * atlas_width) * 4;
            ASSERT(Index < (atlas_width * atlas_height * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*atlas_memory)[Index + c] = texture_memory[j++];
            }
        }
    }
}


static inline u8*
generate_atlas(Arena* arena, 
               aabb2u* aabbs, 
               void* user_datas[],
               u32 aabb_count, 
               u32 width, 
               u32 height) 
{
    u32 atlas_size = width * height * 4;
    u8* atlas_memory = (u8*)Arena_Push_Block(arena, atlas_size);
    if (!atlas_memory) {
        return 0;
    }
    
    for (u32 I = 0; I < aabb_count; ++I) {
        aabb2u aabb = aabbs[I];
        
        auto Type = *(Atlas_Context_Type*)user_datas[I];
        switch(Type) {
            case ATLAS_CONTEXT_TYPE_IMAGE: {
                Arena_Marker scratch = Arena_Mark(arena);
                defer { Arena_Revert(&scratch); };
                
                auto* context = (Atlas_Context_Image*)user_datas[I];
                s32 W, H, C;
                
                Read_File_Result file_mem = {};
                if(!read_file_into_memory(&file_mem,
                                          arena, 
                                          context->filename)){
                    return nullptr;
                }
                
                // TODO: At the moment, there is no clean way for stbi load to 
                // output to a given memory without some really ugly hacks. 
                // so...oh well
                //
                // Github Issue: https://github.com/nothings/stb/issues/58          
                //
                u8* texture_memory = stbi_load_from_memory((u8*)file_mem.data, 
                                                           file_mem.size, 
                                                           &W, &H, &C, 0);
                
                defer { stbi_image_free(texture_memory); };
                write_sub_texture_to_atlas(&atlas_memory, width, height, texture_memory, aabb);
                
            } break;
            case ATLAS_CONTEXT_TYPE_FONT: {
                auto* context = (Atlas_Context_Font*)user_datas[I]; 
                const u32 channels = 4;
                
                s32 W, H;
                u8* font_texture_one_ch = stbtt_GetCodepointTexture(&context->loaded_font->info, 
                                                                    context->raster_scale,
                                                                    context->raster_scale, 
                                                                    context->codepoint, 
                                                                    &W, &H, nullptr, nullptr);
                defer { stbtt_FreeTexture( font_texture_one_ch, nullptr ); };
                
                u32 texture_dimensions = (u32)(W * H);
                if (texture_dimensions == 0) 
                    continue;
                
                Arena_Marker scratch = Arena_Mark(arena);
                u8* font_texture = (u8*)Arena_Push_Block(arena, texture_dimensions*channels); 
                if (!font_texture) {
                    return nullptr;
                }
                defer { Arena_Revert(&scratch); };
                
                u8* font_texture_itr = font_texture;
                for (u32 j = 0, k = 0; j < texture_dimensions; ++j ){
                    for (u32 l = 0; l < channels; ++l ) {
                        font_texture_itr[k++] = font_texture_one_ch[j];
                    }
                }
                write_sub_texture_to_atlas(&atlas_memory, width, height, font_texture, aabb);
                
            } break;
            
        }
        
    }
    
    return atlas_memory;
    
}

#endif //TOOL_BUILD_ASSETS_ATLAS_H
