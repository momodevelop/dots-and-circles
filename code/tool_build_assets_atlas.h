/* date = March 14th 2021 2:41 pm */

#ifndef TOOL_BUILD_ASSETS_ATLAS_H
#define TOOL_BUILD_ASSETS_ATLAS_H


// NOTE(Momo):  Atlas stuff ////////////////////////
enum atlas_context_type {
    AtlasContextType_Image,
    AtlasContextType_Font,
};

struct atlas_context_image {
    atlas_context_type Type;
    const char* Filename;
    image_id Id;
    texture_id TextureId;
    u8* Texture;
};


struct atlas_context_font {
    atlas_context_type Type;
    font_id FontId;
    texture_id TextureId;
    image_id ImageId;
    f32 RasterScale;
    u32 Codepoint;
    loaded_font* LoadedFont;
    u8* Texture;
};    


static inline void 
Tba_WriteSubTextureToAtlas(u8** AtlasMemory, u32 AtlasWidth, u32 AtlasHeight,
                           u8* TextureMemory, aabb2u TextureAabb) 
{
    s32 j = 0;
    u32 TextureAabbW = width(TextureAabb);
    u32 TextureAabbH = height(TextureAabb);
    
    for (u32 y = TextureAabb.min.y; y < TextureAabb.min.y + TextureAabbH; ++y) {
        for (u32 x = TextureAabb.min.x; x < TextureAabb.min.x + TextureAabbW; ++x) {
            u32 Index = (x + y * AtlasWidth) * 4;
            ASSERT(Index < (AtlasWidth * AtlasHeight * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*AtlasMemory)[Index + c] = TextureMemory[j++];
            }
        }
    }
}


static inline u8*
Tba_GenerateAtlas(Arena* arena, 
                  aabb2u* Aabbs, 
                  void* UserDatas[],
                  u32 AabbCount, 
                  u32 Width, 
                  u32 Height) 
{
    u32 AtlasSize = Width * Height * 4;
    u8* AtlasMemory = (u8*)arena->push_block(AtlasSize);
    if (!AtlasMemory) {
        return 0;
    }
    
    for (u32 I = 0; I < AabbCount; ++I) {
        aabb2u Aabb = Aabbs[I];
        
        auto Type = *(atlas_context_type*)UserDatas[I];
        switch(Type) {
            case AtlasContextType_Image: {
                Arena_Mark Scratch = arena->mark();
                defer { Scratch.revert(); };
                
                auto* Context = (atlas_context_image*)UserDatas[I];
                s32 W, H, C;
                
                read_file_result FileMem = {};
                if(!Tba_ReadFileIntoMemory(&FileMem,
                                           arena, 
                                           Context->Filename)){
                    return nullptr;
                }
                
                // TODO: At the moment, there is no clean way for stbi load to 
                // output to a given memory without some really ugly hacks. 
                // so...oh well
                //
                // Github Issue: https://github.com/nothings/stb/issues/58          
                //
                u8* TextureMemory = stbi_load_from_memory((u8*)FileMem.Data, 
                                                          FileMem.Size, 
                                                          &W, &H, &C, 0);
                
                defer { stbi_image_free(TextureMemory); };
                Tba_WriteSubTextureToAtlas(&AtlasMemory, Width, Height, TextureMemory, Aabb);
                
            } break;
            case AtlasContextType_Font: {
                auto* Context = (atlas_context_font*)UserDatas[I]; 
                const u32 Channels = 4;
                
                s32 W, H;
                u8* FontTextureOneCh = stbtt_GetCodepointTexture(&Context->LoadedFont->Info, 
                                                                 Context->RasterScale,
                                                                 Context->RasterScale, 
                                                                 Context->Codepoint, 
                                                                 &W, &H, nullptr, nullptr);
                defer { stbtt_FreeTexture( FontTextureOneCh, nullptr ); };
                
                u32 TextureDimensions = (u32)(W * H);
                if (TextureDimensions == 0) 
                    continue;
                
                Arena_Mark Scratch = arena->mark();
                u8* FontTexture = (u8*)arena->push_block(TextureDimensions*Channels); 
                if (!FontTexture) {
                    return nullptr;
                }
                defer { Scratch.revert(); };
                
                u8* FontTextureItr = FontTexture;
                for (u32 j = 0, k = 0; j < TextureDimensions; ++j ){
                    for (u32 l = 0; l < Channels; ++l ) {
                        FontTextureItr[k++] = FontTextureOneCh[j];
                    }
                }
                Tba_WriteSubTextureToAtlas(&AtlasMemory, Width, Height, FontTexture, Aabb);
                
            } break;
            
        }
        
    }
    
    return AtlasMemory;
    
}

#endif //TOOL_BUILD_ASSETS_ATLAS_H
