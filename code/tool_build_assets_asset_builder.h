/* date = March 14th 2021 2:38 pm */
#ifndef TOOL_BUILD_ASSETS_ASSET_BUILDER_H
#define TOOL_BUILD_ASSETS_ASSET_BUILDER_H

struct tab_asset_builder {
    FILE* File;
    u32 EntryCount;
    long int EntryCountAt;
};

static inline void
Tab_AssetBuilderBegin(tab_asset_builder* Context, 
                      const char* Filename, const char* Signature) 
{
    Context->EntryCount = 0;
    Context->File = nullptr; 
	fopen_s(&Context->File, Filename, "wb");
    Assert(Context->File);
    
    // NOTE(Momo): Write signature
    fwrite(Signature, sizeof(u8), SiStrLen(Signature), Context->File);
    Context->EntryCountAt = ftell(Context->File);
    
    // NOTE(Momo): Reserve space for EntryCount
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
}

static inline void
Tab_AssetBuilderEnd(tab_asset_builder* Context) {
    fseek(Context->File, Context->EntryCountAt, SEEK_SET);
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
    fclose(Context->File);
}

static inline void
Tab_AssetBuilderWriteEntry(tab_asset_builder* Context, game_asset_type AssetType) {
    asset_file_entry Entry = {};
    Entry.Type = AssetType;
    fwrite(&Entry, sizeof(Entry),  1, Context->File);
    ++Context->EntryCount;
}

static inline void 
Tab_AssetBuilderWriteTexture(tab_asset_builder* Context, 
                             texture_id Id, 
                             u32 Width, 
                             u32 Height, 
                             u32 Channels, 
                             u8* Pixels) 
{
    Tab_AssetBuilderWriteEntry(Context, AssetType_Texture);
    
    asset_file_texture Texture = {};
    Texture.Id = Id;
    Texture.Width = Width;
    Texture.Height = Height;
    Texture.Channels = Channels;
    fwrite(&Texture, sizeof(Texture), 1, Context->File);
    
    
    u32 TextureSize = Width * Height * Channels;
    for(u32 i = 0; i < TextureSize; ++i) {
        fwrite(Pixels + i, 1, 1, Context->File);
    }
}


static inline void 
Tab_AssetBuilderWriteTextureFromFile(tab_asset_builder* Context, 
                                     texture_id Id, 
                                     const char* Filename) 
{
    u32 Width = 0, Height = 0, Channels = 0;
    u8* LoadedImage = nullptr;
    {
        i32 W, H, C;
        LoadedImage = stbi_load(Filename, &W, &H, &C, 0);
        Assert(LoadedImage != nullptr);
        
        Width = (u32)W;
        Height = (u32)H; 
        Channels = (u32)C;
    }
    Defer { stbi_image_free(LoadedImage); };
    Tab_AssetBuilderWriteTexture(Context, Id, Width, Height, Channels, LoadedImage);
}


static inline void 
Tab_AssetBuilderWriteAtlasAabb(tab_asset_builder* Context, 
                               atlas_aabb_id Id, 
                               texture_id TargetTextureId, 
                               aabb2u Aabb) 
{
    Tab_AssetBuilderWriteEntry(Context,  AssetType_AtlasAabb);
    
    asset_file_atlas_aabb AtlasAabb = {};
    AtlasAabb.Id = Id;
    AtlasAabb.TextureId = TargetTextureId;
    AtlasAabb.Aabb = Aabb;
    fwrite(&AtlasAabb, sizeof(AtlasAabb), 1,  Context->File);
    printf("%d", TargetTextureId);
}

static inline void
Tab_AssetBuilderWriteFont(tab_asset_builder* Context, 
                          font_id Id, 
                          f32 Ascent, 
                          f32 Descent, 
                          f32 LineGap) 
{
    Tab_AssetBuilderWriteEntry(Context, AssetType_Font);
    
    asset_file_font Font = {};
    Font.Id = Id;
    Font.Ascent = Ascent;
    Font.Descent = Descent;
    Font.LineGap = LineGap;
    fwrite(&Font, sizeof(Font), 1, Context->File);
}

static inline void 
Tab_AssetBuilderWriteFontGlyph(tab_asset_builder* Context, 
                               font_id FontId, 
                               texture_id TargetTextureId, 
                               u32 Codepoint, 
                               f32 Advance, 
                               f32 LeftBearing, 
                               aabb2u AtlasAabb, 
                               aabb2f Box) 
{
    Tab_AssetBuilderWriteEntry(Context, AssetType_FontGlyph);
    
    asset_file_font_glyph FontGlyph = {};
    FontGlyph.FontId = FontId;
    FontGlyph.TextureId = TargetTextureId;
    FontGlyph.Codepoint = Codepoint;
    FontGlyph.AtlasAabb = AtlasAabb;
    FontGlyph.LeftBearing = LeftBearing;
    FontGlyph.Advance = Advance;
    FontGlyph.Box = Box;
    fwrite(&FontGlyph, sizeof(FontGlyph), 1, Context->File);
    
}

static inline void 
Tab_AssetBuilderWriteFontKerning(tab_asset_builder* Context, 
                                 font_id FontId, 
                                 u32 CodepointA,
                                 u32 CodepointB, 
                                 i32 Kerning) 
{
    Tab_AssetBuilderWriteEntry(Context, AssetType_FontKerning);
    
    asset_file_font_kerning Font = {};
    Font.FontId = FontId;
    Font.Kerning = Kerning;
    Font.CodepointA = CodepointA;
    Font.CodepointB = CodepointB;
    fwrite(&Font, sizeof(Font), 1, Context->File);
    
}

// TODO(Momo):
static inline void
Tab_AssetBuilderWriteSound(tab_asset_builder* Context, 
                           game_asset_sound_id SoundId,
                           u32 DataSize,
                           void* Data) 
{
    Tab_AssetBuilderWriteEntry(Context, AssetType_Sound);
    
    asset_file_sound Sound = {};
    Sound.SoundId = SoundId;
    
    
}

#endif //TOOL_BUILD_ASSETS_ASSET_BUILDER_H
