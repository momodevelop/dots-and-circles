#include <stdio.h>
#include <stdlib.h>


#include "ryoji.h"
#include "ryoji_list.h"

#include "game_atlas_types.h"


#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#define DEBUG_PNG 1
#define BITMAP_CHANNELS 4

// NOTE(Momo): File Structures  //////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct atlas_file_header {
    u32 EntryCount;
    u32 EntryDataOffset;
    
    u32 RectCount;
    u32 RectDataOffset;
    
    u32 BitmapWidth;
    u32 BitmapHeight;
    u32 BitmapChannels;
    u32 BitmapDataOffset;
};

struct atlas_file_entry {
    u32 Id;
    u32 Type;
    u32 DataOffset;
};

struct atlas_file_data_image {
    u32 RectIndex;
};

struct atlas_file_data_font {};

#pragma pack(pop)


// NOTE(Momo): Atlas Builder //////////////////////////////////////////////////////////////

struct atlas_builder_entry_image {
    const char* Filename;
    u32 RectIndex;
};

struct atlas_builder_entry {
    atlas_entry_type Type;
    union {
        atlas_builder_entry_image Image;
        // TODO(Momo): Other types here
    };
};

struct atlas_builder_rect {
    u32 X, Y, W, H;
};

struct atlas_builder {
    atlas_builder_rect* Rects; // List
    atlas_builder_rect** SortedRects; // List
    atlas_builder_entry* Entries; // List
    
    u32 Width;
    u32 Height;
    
    const char* OutputFilename;
    b32 Started;
};


static inline u8* 
AllocateBitmap(atlas_builder* Builder) {
    u32 BitmapSize = Builder->Width * Builder->Height * BITMAP_CHANNELS;
    u8* BitmapMemory = (u8*)calloc(BitmapSize, sizeof(u8));
    if (!BitmapMemory) {
        return nullptr;
    }
    
    // NOTE(Momo): Check. Total area of Rects must match 
    
    
    u32 EntryCount = ListCount(Builder->Entries);
    for (u32 i = 0; i < EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        
        // TODO(Momo): Cater for different types
        {
            auto* Image = &Entry->Image;
            auto* Rect = Builder->Rects + Image->RectIndex;
            
            i32 W,H,C;
            
            u8* LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
            if (LoadedImage == nullptr) {
                return nullptr;
            }
            Defer { stbi_image_free(LoadedImage); };
            
            
            Assert((u32)W == Rect->W);
            Assert((u32)H == Rect->H);
            Assert((u32)C == BITMAP_CHANNELS); 
            
            i32 j = 0;
            for (u32 y = Rect->Y; y < Rect->Y + Rect->H; ++y) {
                for (u32 x = Rect->X; x < Rect->X + Rect->W; ++x) {
                    u32 Index = TwoToOne(y, x, Builder->Width) * BITMAP_CHANNELS;
                    Assert(Index < BitmapSize);
                    for (u32 c = 0; c < BITMAP_CHANNELS; ++c) {
                        BitmapMemory[Index + c] = LoadedImage[j++];
                    }
                }
            }
        }
        
    }
    
    return BitmapMemory;
}

static inline void
FreeBitmap(u8* Memory) {
    free(Memory);
}

static inline u32
AddRect(atlas_builder* Builder, u32 W, u32 H) {
    atlas_builder_rect Rect = {0, 0, W, H};
    ListPush(Builder->Rects, Rect);
    
    
    return ListCount(Builder->Rects) - 1;
}

static inline atlas_builder_entry*
AddEntry(atlas_builder* Builder, atlas_builder_entry_type Type) {
    atlas_builder_entry Entry;
    Entry.Type = Type;
    
    ListPush(Builder->Entries, Entry);
    
    return &ListLast(Builder->Entries);
}

static inline void 
AddImage(atlas_builder* Builder, const char* Filename) {
    auto* Entry = AddEntry(Builder,  AtlasEntryType_Image);
    Entry->Image.Filename = Filename;
    
    i32 W, H;
    stbi_info(Filename, &W, &H, 0);
    Entry->Image.RectIndex = AddRect(Builder, (u32)W, (u32)H);
}



// NOTE(Momo): Sort by area. Maybe sort by other methods?
static inline i32
Comparer(const void* Lhs, const void* Rhs) {
    const auto* L = (*(atlas_builder_rect**)(Lhs));
    const auto* R = (*(atlas_builder_rect**)(Rhs));
    
    auto LhsArea = L->W * L->H;
    auto RhsArea = R->W * R->H;
    if (LhsArea != RhsArea)
        return RhsArea - LhsArea;
    
    auto LhsPerimeter = L->W + L->H;
    auto RhsPerimeter = R->W + R->H;
    if (LhsPerimeter != RhsPerimeter)
        return RhsPerimeter - LhsPerimeter;
    
    // By bigger side
    auto LhsBiggerSide = Maximum(L->W, L->H);
    auto RhsBiggerSide = Maximum(R->W, R->H);
    if (LhsBiggerSide != RhsBiggerSide) 
        return RhsBiggerSide - LhsBiggerSide;
    
    // By Width
    if (L->W != R->W)
        return R->W - L->W;
    
    // By right
    if (L->H != R->H)
        return R->H - L->H;
    
    // pathological multipler
    auto LhsMultipler = Maximum(L->W, L->H)/Minimum(L->W, L->H) * L->W * L->H;
    auto RhsMultipler = Maximum(R->W, R->H)/Minimum(R->W, R->H) * R->W * R->H;
    return RhsMultipler - LhsMultipler;
}


static inline b32 
TryPack(atlas_builder* Builder, u32 Width, u32 Height) {
    atlas_builder_rect* Spaces = nullptr;
    {
        atlas_builder_rect Space;
        Space.X = 0;
        Space.Y = 0;
        Space.W = Width;
        Space.H = Height;
        ListPush(Spaces, Space);
    }
    
    
    for (u32 i = 0; i < ListCount(Builder->SortedRects); ++i ) {
        atlas_builder_rect* Rect = *(Builder->SortedRects + i);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 ChosenSpaceIndex = ListCount(Spaces);
        {
            for (u32 j = 0; j < ChosenSpaceIndex ; ++j ) {
                u32 Index = ChosenSpaceIndex - j - 1;
                auto* Space = Spaces + Index;
                // NOTE(Momo): Check if the image fits
                if (Rect->W <= Space->W && Rect->H <= Space->H) {
                    ChosenSpaceIndex = Index;
                    break;
                    
                }
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
        if (ChosenSpaceIndex == ListCount(Spaces)) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        auto ChosenSpace = Spaces[ChosenSpaceIndex];
        if (ListCount(Spaces) > 0) {
            
            Spaces[ChosenSpaceIndex] = ListLast(Spaces);
            ListPop(Spaces);
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Rect->W && ChosenSpace.H == Rect->H) {
            // Split right
            atlas_builder_rect SplitSpaceRight = {};
            SplitSpaceRight.X = ChosenSpace.X + Rect->W;
            SplitSpaceRight.Y = ChosenSpace.Y;
            SplitSpaceRight.W = ChosenSpace.W - Rect->W;
            SplitSpaceRight.H = Rect->H;
            
            ListPush(Spaces, SplitSpaceRight);
            
        }
        else if (ChosenSpace.W == Rect->W && ChosenSpace.H != Rect->H) {
            // Split down
            atlas_builder_rect SplitSpaceDown = {};
            SplitSpaceDown.X = ChosenSpace.X;
            SplitSpaceDown.Y = ChosenSpace.Y + Rect->H;
            SplitSpaceDown.W = ChosenSpace.W;
            SplitSpaceDown.H = ChosenSpace.H - Rect->H;
            
            ListPush(Spaces,SplitSpaceDown);
        }
        else if (ChosenSpace.W != Rect->W && ChosenSpace.H != Rect->H) {
            // Split right
            atlas_builder_rect SplitSpaceRight = {};
            SplitSpaceRight.X = ChosenSpace.X + Rect->W;
            SplitSpaceRight.Y = ChosenSpace.Y;
            SplitSpaceRight.W = ChosenSpace.W - Rect->W;
            SplitSpaceRight.H = Rect->H;
            
            // Split down
            atlas_builder_rect SplitSpaceDown = {};
            SplitSpaceDown.X = ChosenSpace.X;
            SplitSpaceDown.Y = ChosenSpace.Y + Rect->H;
            SplitSpaceDown.W = ChosenSpace.W;
            SplitSpaceDown.H = ChosenSpace.H - Rect->H;
            
            // Choose to insert the bigger one first before the smaller one
            u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
            u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
            if (RightArea > DownArea) {
                ListPush(Spaces, SplitSpaceRight);
                ListPush(Spaces, SplitSpaceDown);
            }
            else {
                ListPush(Spaces, SplitSpaceDown);
                ListPush(Spaces, SplitSpaceRight);
            }
            
        }
        
        // NOTE(Momo): Add to packed images
        Rect->X = ChosenSpace.X;
        Rect->Y = ChosenSpace.Y;
    }
    
    
    return true;
}


static void
Sort(atlas_builder* Builder) {
    printf("\t[Sort] Begin\n");
    
    u32 RectCount = ListCount(Builder->Rects);
    for (u32 i = 0; i < RectCount; ++i) {
        ListPush(Builder->SortedRects, &Builder->Rects[i]);
    }
    qsort(Builder->SortedRects, RectCount, sizeof(atlas_builder_rect*), Comparer);
    
    printf("\t[Sort] End\n");
}

// NOTE(Momo): For now, we only support POT scaling.
static b32
Pack(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
    Assert(StartSize > 0);
    Assert(SizeLimit > 0);
    
    u32 Width = StartSize;
    u32 Height = StartSize;
    
    printf("\t[Pack] Begin\n");
    for (;;) {
        printf("\t\tTrying to pack at: W = %d, H = %d\n", Width, Height);
        b32 Success = TryPack(Builder, Width, Height);
        if (!Success) {
            printf("\t\tFit not found.\n");
            if(Width >= SizeLimit || Height >= SizeLimit) {
                printf("\t\tExceeded size limit! Terminating...\n");
                return false;
            }
        }
        else {
            break;
        }
        
        // NOTE(Momo): POT scaling. 
        Width *= 2;
        Height *= 2;
    }
    printf("\t[Pack] End\n");
    
    Builder->Width = Width;
    Builder->Height = Height;
    
    return true;
}


// NOTE(Momo): GenerateDataFile
// Atlas file will be written in this format:
// - Header
// - Rects
// - Bitmap
// - Entries
// - Bitmap
// - Other data from entries.
static b32 
GenerateDataFile(atlas_builder* Builder) {
    printf("\t[Output] Begin\n");
    printf("\t\tWriting to '%s'... \n", Builder->OutputFilename);
    {
        FILE * OutputFile = fopen(Builder->OutputFilename, "wb");
        if (!OutputFile) {
            printf("\t\tCannot open file '%s'\n", Builder->OutputFilename);
            return false;
        }
        Defer { fclose(OutputFile); };
        
        
        u32 OtherDataAt = 0;
        
        // NOTE(Momo): Write Header
        {
            atlas_file_header Header = {}; 
            
            Header.RectCount = ListCount(Builder->Rects);
            Header.RectDataOffset = sizeof(Header);  
            
            Header.BitmapWidth = Builder->Width;
            Header.BitmapHeight = Builder->Height;
            Header.BitmapChannels = BITMAP_CHANNELS;
            Header.BitmapDataOffset = Header.RectDataOffset + sizeof(atlas_builder_rect) * Header.RectCount;
            
            Header.EntryCount = ListCount(Builder->Entries);
            Header.EntryDataOffset = Header.BitmapDataOffset + Builder->Width * Builder->Height * BITMAP_CHANNELS;
            
            OtherDataAt = Header.EntryDataOffset + sizeof(atlas_file_entry) * Header.RectCount;
            fwrite(&Header, sizeof(Header), 1, OutputFile);
        }
        
        // NOTE(Momo): Write Rects
        {
            u32 RectCount = ListCount(Builder->Rects);
            fwrite(Builder->Rects, sizeof(atlas_builder_rect), RectCount, OutputFile);
        }
        
        // NOTE(Momo): Write Bitmap
        {
            u8 * BitmapMemory = AllocateBitmap(Builder);
            if (BitmapMemory == nullptr) {
                printf("\t\tBitmap allocation failed\n");
                return false;
                
            }
            Defer { FreeBitmap(BitmapMemory); };
            u32 BitmapSize = Builder->Width * Builder->Height * BITMAP_CHANNELS;
            
            fwrite(BitmapMemory, 1, BitmapSize, OutputFile);
            
#if DEBUG_PNG
            {
                // TODO(Momo): Use dynamic string
                //char Buffer[256] = {};
                const char * PngFilename = "debug.png";
                printf("\t\tWriting to '%s'... \n", PngFilename);
                if (!stbi_write_png(PngFilename, Builder->Width, Builder->Height, BITMAP_CHANNELS, BitmapMemory, Builder->Width * BITMAP_CHANNELS)) {
                    printf("\t\tWrite failed\n");
                    return false;
                }
                printf("\t\tWrite to '%s' completed\n", PngFilename);
            }
#endif
        }
        
        // NOTE(Momo): Write Entries
        {
            usize EntryAt = ftell(OutputFile);
            usize DataAt = EntryAt + (sizeof(atlas_file_entry) * ListCount(Builder->Entries));
            
            for (u32 i = 0; i < ListCount(Builder->Entries); ++i) {
                auto* Entry = Builder->Entries + i;
                
                // NOTE(Momo): Write file entry
                fseek(OutputFile, (i32)EntryAt, SEEK_SET);
                {
                    atlas_file_entry FileEntry = {};
                    FileEntry.Id = i;
                    FileEntry.Type = Entry->Type;
                    FileEntry.DataOffset = (u32)DataAt;
                    
                    fwrite(&FileEntry, sizeof(atlas_file_entry), 1, OutputFile); 
                    EntryAt += sizeof(atlas_file_entry);
                }
                
                fseek(OutputFile, (i32)DataAt, SEEK_SET);
                {
                    // NOTE(Momo): Write Data
                    switch(Entry->Type) {
                        case AtlasEntryType_Image: {
                            atlas_file_data_image ImageData = {};
                            ImageData.RectIndex = Entry->Image.RectIndex;
                            
                            fwrite(&ImageData, sizeof(atlas_file_data_image), 1, OutputFile); 
                            DataAt += sizeof(atlas_file_data_image);
                        } break;
                        default: {
                            Assert(false);
                        }
                    }
                }
                
            }
            
        }
        
        
        
    }
    
    printf("\t\tWrite to '%s' completed\n", Builder->OutputFilename);
    printf("\t[Output] End\n");
    
    return true;
}

static b32 
Build(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
    
    
    return true;
}


static inline void
StartAtlas(atlas_builder* Builder, const char* OutputFilename) {
    Assert(!Builder->Started);
    atlas_builder NewBuilder = {};
    (*Builder) = NewBuilder;
    Builder->OutputFilename = OutputFilename;
    Builder->Started = true;
    
}

static inline void
EndAtlas(atlas_builder* Builder) {
    printf("[Build] Building '%s' started \n", Builder->OutputFilename);
    Sort(Builder);
    Pack(Builder, 128, 4096);
    GenerateDataFile(Builder);
    printf("[Build] Building '%s' complete\n", Builder->OutputFilename);
    
    
    Assert(Builder->Started);
    ListFree(Builder->Rects);
    ListFree(Builder->Entries);
    Builder->Started = false;
}



int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    
    StartAtlas(Atlas, "assets/yuu.atlas");
    {
        
#if 1
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
#else
        AddImage(Atlas, "assets/atlas_test/1.png");
        AddImage(Atlas, "assets/atlas_test/2.png");
        AddImage(Atlas, "assets/atlas_test/3.png");
        AddImage(Atlas, "assets/atlas_test/4.png");
        AddImage(Atlas, "assets/atlas_test/5.png");
        AddImage(Atlas, "assets/atlas_test/6.png");
        AddImage(Atlas, "assets/atlas_test/7.png");
        AddImage(Atlas, "assets/atlas_test/8.png");
        AddImage(Atlas, "assets/atlas_test/9.png");
        AddImage(Atlas, "assets/atlas_test/10.png");
        for (u32 i = 0; i < 50; ++i) {
            AddImage(Atlas, "assets/atlas_test/2.png");
            AddImage(Atlas, "assets/atlas_test/3.png");
            AddImage(Atlas, "assets/atlas_test/4.png");
            AddImage(Atlas, "assets/atlas_test/5.png");
            AddImage(Atlas, "assets/atlas_test/6.png");
            AddImage(Atlas, "assets/atlas_test/7.png");
            AddImage(Atlas, "assets/atlas_test/8.png");
            AddImage(Atlas, "assets/atlas_test/9.png");
        }
#endif
        //AddFont(Atlas, "assets/font.odt", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 36.f);
    }
    EndAtlas(Atlas);
    
    
    
    return 0;
    
}