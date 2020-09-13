#if 0

#ifndef GAME_ASSET_FONT_H
#define GAME_ASSET_FONT_H

struct font_character {
    u32 X, Y, W, H;
    u32 Codepoint;
};

struct font {
    u32 Width;
    u32 Height;
    void* Pixels;
    
    u32 BitmapId;
    
    font_character* CharacterData;
    u32 CharacterCount;
};

struct font_id { u32 Value; };


static inline void
LoadFont(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data) {
    auto* YuuFont = Read<yuu_font>(&Data);
    
    // NOTE(Momo): Allocate the font
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Font;
        Entry->Id = Id;
        Entry->Font = PushStruct<font>(&Assets->Arena);
        Entry->Font->Width = YuuFont->Width;
        Entry->Font->Height = YuuFont->Height;
        Entry->Font->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate character data
        Entry->Font->CharacterData = PushArray<font_character>(&Assets->Arena, YuuFont->CharacterCount);
        Entry->Font->CharacterCount = YuuFont->CharacterCount;
        for (u32 i = 0; i < CharacterCount; ++i) {
            auto* CharacterData = Entry->Font->CharacterData + i;
            
            u32 X = Read32<u32>(&Data, false);
            u32 Y = Read32<u32>(&Data, false);
            u32 W = Read32<u32>(&Data, false);
            u32 H = Read32<u32>(&Data, false);
            u32 Codepoint = Read32<u32>(&Data, false);
            
            CharacterData->X = X;
            CharacterData->Y = Y;
            CharacterData->W = W;
            CharacterData->H = H;
            CharacterData->Codepoint = Codepoint;
        }
        
        
        // NOTE(Momo): Allocate pixel data
        usize Size = Width * Height * Channels;
        Entry->Font->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Font->Pixels);
        CopyBlock(Entry->Font->Pixels, Data, Size);
        
        PushCommandLinkTexture(RenderCommands,
                               Entry->Font->Width, 
                               Entry->Font->Height,
                               Entry->Font->Pixels,
                               Entry->Font->BitmapId);
    }
}


// NOTE(Momo): Font Interface
static inline font_id 
GetFont(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Font);
    return { Id };
}


static inline u32 
GetBitmapId(game_assets* Assets, font_id Id) {
    asset_entry* Entry = Assets->Entries + Id.Value;
    return Entry->Font->BitmapId;
}


// TODO(Momo): This is very bad haha. 
// We should really use Codepoint to get the font UV..........
static inline rect2f 
GetFontCharacterUV(game_assets* Assets, font_id Id, u32 FrameIndex) {
    asset_entry* Entry = Assets->Entries + Id.Value;
    Assert(Entry->Type == AssetType_Font);
    
    auto* Font = Entry->Font;
    auto* CharacterData = Font->CharacterData + FrameIndex;
    
    f32 UVXmin = (f32)CharacterData->X / Font->Width;
    f32 UVYmin = 1.f - ((f32)CharacterData->H / Font->Height); ;
    f32 UVXmax = UVXmin + ((f32) CharacterData->W / Font->Width);
    f32 UVYmax = 1.f;
    
    return {
        UVXmin,
        UVYmin,
        UVXmax,
        UVYmax,
    };
}


#endif //GAME_ASSET_FONT_H
#endif