#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_asset_types.h"


struct asset_data_font_character_data {
    u32 X, Y, W, H;
    u32 Codepoint;
};

struct asset_data_font {
    u32 Width;
    u32 Height;
    void* Pixels;
    
    u32 BitmapId;
    
    asset_data_font_character_data* CharacterData;
    u32 CharacterCount;
};

struct asset_data_image {
    u32 Width;
    u32 Height;
    void* Pixels; 
    u32 BitmapId;
};

struct asset_data_spritesheet {
    u32 Width;
    u32 Height;
    void* Pixels; 
    
    rect2f* Frames;
    u32 Rows;
    u32 Cols;
    
    u32 BitmapId;
};

struct image_id { 
    u32 Value; 
    operator u32() { return Value; }
};

struct spritesheet_id { 
    u32 Value; 
    operator u32() { return Value; }
};

struct font_id { 
    u32 Value; 
    operator u32() { return Value; }
};



struct asset_entry {
    asset_type Type;
    asset_id Id;
    union {
        void* Data;
        asset_data_image* Image;
        asset_data_spritesheet* Spritesheet;
        asset_data_font* Font;
    };
};

struct game_assets {
    memory_arena Arena;
    asset_entry* Entries;
    u32 EntryCount;
    
    u32 BitmapCounter;
    
    platform_api* Platform;
};

static inline b32
CheckAssetSignature(void *Memory) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 i = 0; i < ArrayCount(AssetSignature); ++i) {
        if (MemoryU8[i] != AssetSignature[i]) {
            return false;
        }
    }
    return true;
}


static inline void
LoadImage(game_assets* Assets, commands* RenderCommands, asset_id Id, u8** Data) {
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    
    // NOTE(Momo): Allocate Image
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Image;
        Entry->Id = Id;
        Entry->Image = PushStruct<asset_data_image>(&Assets->Arena);
        Entry->Image->Width = Width;
        Entry->Image->Height = Height;
        Entry->Image->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate pixel data
        usize Size = Width * Height * Channels;
        Entry->Image->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Image->Pixels);
        CopyBlock(Entry->Image->Pixels, (*Data), Size);
        
        PushCommandLinkTexture(RenderCommands, 
                               Entry->Image->Width, 
                               Entry->Image->Height,
                               Entry->Image->Pixels,
                               Entry->Image->BitmapId);
    }
}

static inline void
LoadFont(game_assets* Assets, commands* RenderCommands, asset_id Id, u8** Data) {
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    u32 CharacterCount = Read32<u32>(Data, false);
    
    
    // NOTE(Momo): Allocate the font
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Font;
        Entry->Id = Id;
        Entry->Font = PushStruct<asset_data_font>(&Assets->Arena);
        Entry->Font->Width = Width;
        Entry->Font->Height = Height;
        Entry->Font->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate character data
        Entry->Font->CharacterData = PushArray<asset_data_font_character_data>(&Assets->Arena, CharacterCount);
        Entry->Font->CharacterCount = CharacterCount;
        for (u32 i = 0; i < CharacterCount; ++i) {
            auto* CharacterData = Entry->Font->CharacterData + i;
            
            u32 X = Read32<u32>(Data, false);
            u32 Y = Read32<u32>(Data, false);
            u32 W = Read32<u32>(Data, false);
            u32 H = Read32<u32>(Data, false);
            u32 Codepoint = Read32<u32>(Data, false);
            
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
        CopyBlock(Entry->Font->Pixels, (*Data), Size);
        
        PushCommandLinkTexture(RenderCommands,
                               Entry->Font->Width, 
                               Entry->Font->Height,
                               Entry->Font->Pixels,
                               Entry->Font->BitmapId);
    }
    
}

static inline void
LoadSpritesheet(game_assets* Assets, commands* RenderCommands, asset_id Id, u8** Data) 
{
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    u32 Rows = Read32<u32>(Data, false);
    u32 Cols = Read32<u32>(Data, false);
    
    // NOTE(Momo): Allocate Spritesheet
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Spritesheet;
        Entry->Id = Id;
        Entry->Spritesheet = PushStruct<asset_data_spritesheet>(&Assets->Arena);
        Entry->Spritesheet->Width = Width;
        Entry->Spritesheet->Height = Height;
        Entry->Spritesheet->Rows = Rows;
        Entry->Spritesheet->Cols = Cols; 
        Entry->Spritesheet->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate pixel data
        usize Size = Width * Height * Channels;
        Entry->Spritesheet->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Spritesheet->Pixels);
        CopyBlock(Entry->Spritesheet->Pixels, (*Data), Size);
        
        f32 FrameWidth = (f32)Width/Cols;
        f32 FrameHeight = (f32)Height/Rows;
        f32 CellWidth = FrameWidth/Width;
        f32 CellHeight = FrameHeight/Height;
        f32 HalfPixelWidth =  0.25f/Width;
        f32 HalfPixelHeight = 0.25f/Height;
        
        // NOTE(Momo): Allocate sprite frames
        u32 TotalFrames = Rows * Cols;
        rect2f* Rects = PushArray<rect2f>(&Assets->Arena, TotalFrames); 
        for (u8 r = 0; r < Rows; ++r) {
            for (u8 c = 0; c < Cols; ++c) {
                Rects[TwoToOne(r,c,Cols)] = { 
                    c * CellWidth + HalfPixelWidth,
                    r * CellHeight + HalfPixelHeight,
                    (c+1) * CellWidth - HalfPixelWidth,
                    (r+1) * CellHeight - HalfPixelHeight,
                };
            }
        }
        Entry->Spritesheet->Frames = Rects;
        
        PushCommandLinkTexture(RenderCommands,
                               Entry->Spritesheet->Width, 
                               Entry->Spritesheet->Height,
                               Entry->Spritesheet->Pixels,
                               Entry->Spritesheet->BitmapId);
    }
}


// TODO(Momo): Perhaps remove render_commands and replace with platform call for linking textures?
static inline void
Init(game_assets* Assets, 
     memory_arena* Arena, 
     platform_api* Platform,
     commands* RenderCommands,
     const char* Filename) 
{
    memory_arena* AssetArena = &Assets->Arena;
    SubArena(AssetArena, Arena, Megabytes(10));
    Assets->Platform = Platform;
    
    auto Scratch = BeginTempArena(AssetArena);
    Defer{ EndTempArena(Scratch); };
    
    // NOTE(Momo): File read into temp arena
    u8* FileMemory = nullptr;
    u8* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename);
        Assert(Filesize);
        FileMemory = FileMemoryItr = (u8*)PushBlock(&Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename);
    }
    
    // NOTE(Momo): Read and check file header
    {
        Assert(CheckAssetSignature(FileMemoryItr));
        FileMemoryItr+= ArrayCount(AssetSignature);
        
        // NOTE(Momo): Read the counts in order
        Assets->EntryCount = Read32<u32>(&FileMemoryItr, false);
    }
    
    // NOTE(Momo): Allocate Assets
    {
        // NOTE(Momo): All entries are allocated first. Their data will follow after.
        Assets->Entries = PushArray<asset_entry>(&Assets->Arena, Assets->EntryCount);
        
        for (u32 i = 0; i < Assets->EntryCount; ++i)
        {
            // NOTE(Momo): Read headers
            auto FileAssetType = Read32<asset_type>(&FileMemoryItr, false);
            u32 FileOffsetToEntry = Read32<u32>(&FileMemoryItr, false);
            auto FileAssetId = Read32<asset_id>(&FileMemoryItr, false);
            
            u8* FileEntryItr = FileMemory + FileOffsetToEntry;
            
            switch(FileAssetType) {
                case AssetType_Image: {
                    LoadImage(Assets, RenderCommands, FileAssetId, &FileEntryItr);
                } break;
                case AssetType_Font: {
                    LoadFont(Assets, RenderCommands, FileAssetId, &FileEntryItr);
                } break;
                case AssetType_Spritesheet: {
                    LoadSpritesheet(Assets, RenderCommands, FileAssetId, &FileEntryItr);
                } break;
                case AssetType_Sound: {
                    // TODO(Momo): Implement
                } break;
                
            }
            
        }
    }
    
}



// NOTE(Momo): Image Interface
static inline image_id
GetImage(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return { Id };
}


static inline u32 
GetBitmapId(game_assets* Assets, image_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return Entry->Image->BitmapId;
}

// NOTE(Momo): Spritesheet Interface
static inline spritesheet_id
GetSpritesheet(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Spritesheet);
    return { Id };
}

static inline u32 
GetBitmapId(game_assets* Assets, spritesheet_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Spritesheet);
    return Entry->Spritesheet->BitmapId;
}

static inline rect2f 
GetSpritesheetFrame(game_assets* Assets, spritesheet_id Id, u32 FrameIndex) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Spritesheet);
    Assert(FrameIndex < (Entry->Spritesheet->Rows * Entry->Spritesheet->Cols));
    return Entry->Spritesheet->Frames[FrameIndex];
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
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Font);
    return Entry->Font->BitmapId;
}

// TODO(Momo): This is very bad haha. We should really use Codepoint to get the font UV..........
static inline rect2f 
GetFontUV(game_assets* Assets, font_id Id, u32 FrameIndex) {
    asset_entry* Entry = Assets->Entries + Id;
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

#endif  