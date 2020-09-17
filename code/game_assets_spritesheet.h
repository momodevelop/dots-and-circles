#ifndef GAME_ASSET_SPRITESHEET_H
#define GAME_ASSET_SPRITESHEET_H

struct spritesheet {
    u32 Width;
    u32 Height;
    u32 Channels;
    u32 Rows;
    u32 Cols;
    u32 BitmapId;
    
    void* Pixels; 
    rect2f* Frames;
};

struct spritesheet_id { u32 Value; };

static inline void
LoadSpritesheet(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data) 
{
    auto* YuuSS = Read<yuu_spritesheet>(&Data);
    
    // NOTE(Momo): Allocate Spritesheet
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Spritesheet;
        
        Entry->Spritesheet = PushStruct<spritesheet>(&Assets->Arena);
        spritesheet* SS = Entry->Spritesheet;
        
        SS->Width = YuuSS->Width;
        SS->Height = YuuSS->Height;
        SS->Channels = YuuSS->Channels;
        SS->Rows = YuuSS->Rows;
        SS->Cols = YuuSS->Cols; 
        SS->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate pixel data
        usize Size = SS->Width * SS->Height * SS->Channels;
        SS->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(SS->Pixels);
        CopyBlock(SS->Pixels, Data, Size);
        
        
        f32 FrameWidth = (f32)SS->Width/SS->Cols;
        f32 FrameHeight = (f32)SS->Height/SS->Rows;
        f32 CellWidth = FrameWidth/SS->Width;
        f32 CellHeight = FrameHeight/SS->Height;
        f32 HalfPixelWidth =  0.25f/SS->Width;
        f32 HalfPixelHeight = 0.25f/SS->Height;
        
        // NOTE(Momo): Allocate sprite frames
        u32 TotalFrames = SS->Rows * SS->Cols;
        rect2f* Rects = PushArray<rect2f>(&Assets->Arena, TotalFrames); 
        for (u8 r = 0; r < SS->Rows; ++r) {
            for (u8 c = 0; c < SS->Cols; ++c) {
                Rects[TwoToOne(r,c,SS->Cols)] = { 
                    c * CellWidth + HalfPixelWidth,
                    r * CellHeight + HalfPixelHeight,
                    (c+1) * CellWidth - HalfPixelWidth,
                    (r+1) * CellHeight - HalfPixelHeight,
                };
            }
        }
        SS->Frames = Rects;
        
        PushCommandLinkTexture(RenderCommands,
                               SS->Width, 
                               SS->Height,
                               SS->Pixels,
                               SS->BitmapId);
    }
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
    asset_entry* Entry = Assets->Entries + Id.Value;
    return Entry->Spritesheet->BitmapId;
}

static inline rect2f 
GetSpritesheetFrame(game_assets* Assets, spritesheet_id Id, u32 FrameIndex) {
    asset_entry* Entry = Assets->Entries + Id.Value;
    Assert(FrameIndex < (Entry->Spritesheet->Rows * Entry->Spritesheet->Cols));
    return Entry->Spritesheet->Frames[FrameIndex];
}


#endif //GAME_ASSET_SPRITESHEET_H
