/* date = April 30th 2021 10:59 am */

#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

struct tba_context {
    FILE* File;
    u32 EntryCount;
    long int EntryCountAt;
};

static inline void
Tba_Begin(tba_context* Context, 
          const char* Filename, 
          const char* Signature) 
{
    Context->EntryCount = 0;
    Context->File = nullptr; 
	fopen_s(&Context->File, Filename, "wb");
    ASSERT(Context->File);
    
    // NOTE(Momo): Write signature
    fwrite(Signature, sizeof(u8), cstr_length(Signature), Context->File);
    Context->EntryCountAt = ftell(Context->File);
    
    // NOTE(Momo): Reserve space for EntryCount
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
}

static inline void
Tba_End(tba_context* Context) 
{
    fseek(Context->File, Context->EntryCountAt, SEEK_SET);
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
    fclose(Context->File);
}

static inline void
Tba_WriteEntry(tba_context* Context, Asset_Type AssetType) 
{
    Asset_File_Entry Entry = {};
    Entry.Type = AssetType;
    fwrite(&Entry, sizeof(Entry),  1, Context->File);
    ++Context->EntryCount;
}

static inline void 
Tba_WriteTexture(tba_context* Context, 
                 Texture_ID Id, 
                 u32 Width, 
                 u32 Height, 
                 u32 channels, 
                 u8* Pixels) 
{
    Tba_WriteEntry(Context, ASSET_TYPE_TEXTURE);
    
    Asset_File_Texture Texture = {};
    Texture.Id = Id;
    Texture.Width = Width;
    Texture.Height = Height;
    Texture.channels = channels;
    fwrite(&Texture, sizeof(Texture), 1, Context->File);
    
    
    u32 TextureSize = Width * Height * channels;
    for(u32 i = 0; i < TextureSize; ++i) {
        fwrite(Pixels + i, 1, 1, Context->File);
    }
}


static inline void 
Tba_WriteTextureFromFile(tba_context* Context, 
                         Texture_ID Id, 
                         const char* Filename) 
{
    u32 Width = 0, Height = 0, channels = 0;
    u8* LoadedImage = nullptr;
    {
        s32 W, H, C;
        LoadedImage = stbi_load(Filename, &W, &H, &C, 0);
        ASSERT(LoadedImage != nullptr);
        
        Width = (u32)W;
        Height = (u32)H; 
        channels = (u32)C;
    }
    defer { stbi_image_free(LoadedImage); };
    Tba_WriteTexture(Context, Id, Width, Height, channels, LoadedImage);
}


static inline void 
Tba_WriteImage(tba_context* Context, 
               Image_ID Id, 
               Texture_ID TargetTextureId, 
               aabb2u Aabb) 
{
    Tba_WriteEntry(Context,  ASSET_TYPE_IMAGE);
    
    Asset_File_Image Image = {};
    Image.Id = Id;
    Image.TextureId = TargetTextureId;
    Image.Aabb = Aabb;
    fwrite(&Image, sizeof(Image), 1,  Context->File);
}

static inline void
Tba_WriteFont(tba_context* Context, 
              Font_ID Id, 
              f32 Ascent, 
              f32 Descent, 
              f32 LineGap) 
{
    Tba_WriteEntry(Context, ASSET_TYPE_FONT);
    
    Asset_File_Font Font = {};
    Font.Id = Id;
    Font.Ascent = Ascent;
    Font.Descent = Descent;
    Font.LineGap = LineGap;
    fwrite(&Font, sizeof(Font), 1, Context->File);
}

static inline void 
Tba_WriteFontGlyph(tba_context* Context, 
                   Font_ID FontId, 
                   Image_ID ImageId,
                   Texture_ID TextureId, 
                   u32 Codepoint, 
                   f32 Advance, 
                   f32 LeftBearing, 
                   aabb2u ImageAabb, 
                   aabb2f Box) 
{
    Tba_WriteImage(Context, ImageId, TextureId, ImageAabb);
    
    Tba_WriteEntry(Context, ASSET_TYPE_FONT_GLYPH);
    
    Asset_File_Font_Glyph FontGlyph = {};
    FontGlyph.FontId = FontId;
    FontGlyph.ImageId = ImageId;
    FontGlyph.Codepoint = Codepoint;
    FontGlyph.LeftBearing = LeftBearing;
    FontGlyph.Advance = Advance;
    FontGlyph.Box = Box;
    fwrite(&FontGlyph, sizeof(FontGlyph), 1, Context->File);
    
}

static inline void 
Tba_WriteFontKerning(tba_context* Context, 
                     Font_ID FontId, 
                     u32 CodepointA,
                     u32 CodepointB, 
                     s32 Kerning) 
{
    Tba_WriteEntry(Context, ASSET_TYPE_FONT_KERNING);
    
    Asset_File_Font_Kerning Font = {};
    Font.FontId = FontId;
    Font.Kerning = Kerning;
    Font.CodepointA = CodepointA;
    Font.CodepointB = CodepointB;
    fwrite(&Font, sizeof(Font), 1, Context->File);
    
}

static inline void 
Tba_WriteAnime(tba_context* Context, 
               Anime_ID AnimeId, 
               Image_ID* Frames,
               u32 FrameCount) 
{
    Tba_WriteEntry(Context, ASSET_TYPE_ANIME);
    
    Asset_File_Anime Anime = {};
    Anime.Id = AnimeId;
    Anime.FrameCount = FrameCount;
    
    fwrite(&Anime, sizeof(Anime), 1, Context->File);
    fwrite(Frames, sizeof(Image_ID), FrameCount, Context->File);
}

static inline void 
Tba_WriteMsg(tba_context* Context, 
             Msg_ID MsgId,
             const char* Message) 
{
    Tba_WriteEntry(Context, ASSET_TYPE_MSG);
    
    Asset_File_Msg Msg = {};
    Msg.Id = MsgId;
    Msg.Count = cstr_length(Message) - 1;
    
    fwrite(&Msg, sizeof(Msg), 1, Context->File);
    fwrite(Message, sizeof(char), Msg.Count, Context->File);
}

static inline void
Tba_WriteWav(tba_context* Context, 
             Sound_ID SoundId,
             Wav_Load_Result* WavResult) 
{
    // We restrict the type of sound the game allows here
    ASSERT(WavResult->fmt_chunk.num_channels == Game_AudioChannels);
    ASSERT(WavResult->fmt_chunk.sample_rate == Game_AudioSamplesPerSecond);
    ASSERT(WavResult->fmt_chunk.bits_per_sample == Game_AudioBitsPerSample);
    
    Tba_WriteEntry(Context, ASSET_TYPE_SOUND);
    
    Asset_File_Sound Sound = {};
    Sound.Id = SoundId;
    Sound.DataCount = WavResult->data_chunk.size / sizeof(s16);
    
    fwrite(&Sound, sizeof(Sound), 1, Context->File);
    fwrite(WavResult->data, sizeof(s16), Sound.DataCount, Context->File);
    
}

#endif //TOOL_BUILD_ASSETS_H
