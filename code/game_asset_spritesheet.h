#ifndef GAME_ASSET_SPRITESHEET_H
#define GAME_ASSET_SPRITESHEET_H

struct spritesheet {
    u32 Width;
    u32 Height;
    void* Pixels; 
    
    rect2f* Frames;
    u32 Rows;
    u32 Cols;
    
    u32 BitmapId;
};

struct spritesheet_id { u32 Value; };

static inline void
LoadSpritesheet(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data) 
{
    u32 Width = Read32<u32>(&Data, false);
    u32 Height = Read32<u32>(&Data, false);
    u32 Channels = Read32<u32>(&Data, false);
    u32 Rows = Read32<u32>(&Data, false);
    u32 Cols = Read32<u32>(&Data, false);
    
    
    // NOTE(Momo): Allocate Spritesheet
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Spritesheet;
        Entry->Id = Id;
        Entry->Spritesheet = PushStruct<spritesheet>(&Assets->Arena);
        Entry->Spritesheet->Width = Width;
        Entry->Spritesheet->Height = Height;
        Entry->Spritesheet->Rows = Rows;
        Entry->Spritesheet->Cols = Cols; 
        Entry->Spritesheet->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate pixel data
        usize Size = Width * Height * Channels;
        Entry->Spritesheet->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Spritesheet->Pixels);
        CopyBlock(Entry->Spritesheet->Pixels, Data, Size);
        
        
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
