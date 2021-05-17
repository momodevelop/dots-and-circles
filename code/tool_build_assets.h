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
    Assert(Context->File);
    
    // NOTE(Momo): Write signature
    fwrite(Signature, sizeof(u8), SiStrLen(Signature), Context->File);
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
Tba_WriteEntry(tba_context* Context, asset_type AssetType) 
{
    asset_file_entry Entry = {};
    Entry.Type = AssetType;
    fwrite(&Entry, sizeof(Entry),  1, Context->File);
    ++Context->EntryCount;
}

static inline void 
Tba_WriteTexture(tba_context* Context, 
                 texture_id Id, 
                 u32 Width, 
                 u32 Height, 
                 u32 Channels, 
                 u8* Pixels) 
{
    Tba_WriteEntry(Context, AssetType_Texture);
    
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
Tba_WriteTextureFromFile(tba_context* Context, 
                         texture_id Id, 
                         const char* Filename) 
{
    u32 Width = 0, Height = 0, Channels = 0;
    u8* LoadedImage = nullptr;
    {
        s32 W, H, C;
        LoadedImage = stbi_load(Filename, &W, &H, &C, 0);
        Assert(LoadedImage != nullptr);
        
        Width = (u32)W;
        Height = (u32)H; 
        Channels = (u32)C;
    }
    Defer { stbi_image_free(LoadedImage); };
    Tba_WriteTexture(Context, Id, Width, Height, Channels, LoadedImage);
}


static inline void 
Tba_WriteImage(tba_context* Context, 
               image_id Id, 
               texture_id TargetTextureId, 
               aabb2u Aabb) 
{
    Tba_WriteEntry(Context,  AssetType_Image);
    
    asset_file_image Image = {};
    Image.Id = Id;
    Image.TextureId = TargetTextureId;
    Image.Aabb = Aabb;
    fwrite(&Image, sizeof(Image), 1,  Context->File);
}

static inline void
Tba_WriteFont(tba_context* Context, 
              font_id Id, 
              f32 Ascent, 
              f32 Descent, 
              f32 LineGap) 
{
    Tba_WriteEntry(Context, AssetType_Font);
    
    asset_file_font Font = {};
    Font.Id = Id;
    Font.Ascent = Ascent;
    Font.Descent = Descent;
    Font.LineGap = LineGap;
    fwrite(&Font, sizeof(Font), 1, Context->File);
}

static inline void 
Tba_WriteFontGlyph(tba_context* Context, 
                   font_id FontId, 
                   image_id ImageId,
                   texture_id TextureId, 
                   u32 Codepoint, 
                   f32 Advance, 
                   f32 LeftBearing, 
                   aabb2u ImageAabb, 
                   aabb2f Box) 
{
    Tba_WriteImage(Context, ImageId, TextureId, ImageAabb);
    
    Tba_WriteEntry(Context, AssetType_FontGlyph);
    
    asset_file_font_glyph FontGlyph = {};
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
                     font_id FontId, 
                     u32 CodepointA,
                     u32 CodepointB, 
                     s32 Kerning) 
{
    Tba_WriteEntry(Context, AssetType_FontKerning);
    
    asset_file_font_kerning Font = {};
    Font.FontId = FontId;
    Font.Kerning = Kerning;
    Font.CodepointA = CodepointA;
    Font.CodepointB = CodepointB;
    fwrite(&Font, sizeof(Font), 1, Context->File);
    
}

static inline void 
Tba_WriteAnime(tba_context* Context, 
               anime_id AnimeId, 
               image_id* Frames,
               u32 FrameCount) 
{
    Tba_WriteEntry(Context, AssetType_Anime);
    
    asset_file_anime Anime = {};
    Anime.Id = AnimeId;
    Anime.FrameCount = FrameCount;
    
    fwrite(&Anime, sizeof(Anime), 1, Context->File);
    fwrite(Frames, sizeof(image_id), FrameCount, Context->File);
}

static inline void 
Tba_WriteMsg(tba_context* Context, 
             msg_id MsgId,
             const char* Message) 
{
    Tba_WriteEntry(Context, AssetType_Message);
    
    asset_file_msg Msg = {};
    Msg.Id = MsgId;
    Msg.Count = SiStrLen(Message) - 1;
    
    fwrite(&Msg, sizeof(Msg), 1, Context->File);
    fwrite(Message, sizeof(char), Msg.Count, Context->File);
}

static inline void
Tba_WriteWav(tba_context* Context, 
             sound_id SoundId,
             wav_load_result* WavResult) 
{
    // We restrict the type of sound the game allows here
    Assert(WavResult->FmtChunk.NumChannels == Game_AudioChannels);
    Assert(WavResult->FmtChunk.SampleRate == Game_AudioSamplesPerSecond);
    Assert(WavResult->FmtChunk.BitsPerSample == Game_AudioBitsPerSample);
    
    Tba_WriteEntry(Context, AssetType_Sound);
    
    asset_file_sound Sound = {};
    Sound.Id = SoundId;
    Sound.DataCount = WavResult->DataChunk.Size / sizeof(s16);
    
    fwrite(&Sound, sizeof(Sound), 1, Context->File);
    fwrite(WavResult->Data, sizeof(s16), Sound.DataCount, Context->File);
    
}

#endif //TOOL_BUILD_ASSETS_H
