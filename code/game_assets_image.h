#ifndef GAME_ASSET_IMAGE_H
#define GAME_ASSET_IMAGE_H

struct image {
    u32 Width;
    u32 Height;
    u32 Channels;
    void* Pixels; 
    u32 BitmapId;
};

struct image_id { u32 Value; };

static inline void
LoadImage(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data) {
    auto* YuuImage = Read<yuu_image>(&Data);
    
    // NOTE(Momo): Allocate Image
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Image;
        Entry->Image = PushStruct<image>(&Assets->Arena);
        Entry->Image->Width = YuuImage->Width;
        Entry->Image->Height = YuuImage->Height;
        Entry->Image->Channels = YuuImage->Channels;
        Entry->Image->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate pixel data
        usize Size = Entry->Image->Width * Entry->Image->Height * Entry->Image->Channels;
        Entry->Image->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Image->Pixels);
        CopyBlock(Entry->Image->Pixels, Data, Size);
        
        PushCommandLinkTexture(RenderCommands, 
                               Entry->Image->Width, 
                               Entry->Image->Height,
                               Entry->Image->Pixels,
                               Entry->Image->BitmapId);
    }
}

// NOTE(Momo): Image Interface
static inline image_id
GetImage(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return { Id };
}

static inline image*
GetImagePtr(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return Entry->Image;
}

static inline u32 
GetBitmapId(game_assets* Assets, image_id Id) {
    asset_entry* Entry = Assets->Entries + Id.Value;
    return Entry->Image->BitmapId;
}

#endif //GAME_ASSET_IMAGE_H