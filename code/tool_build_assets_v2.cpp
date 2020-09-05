#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


struct rect {
    i32 X, Y, W, H;
};

struct image_entry {
    const char* Filename;
    i32 X, Y, W, H;
};


struct image_pack {
    image_entry Entries[1024];
    u32 EntryCount;
};

struct packed_images {
};


static inline void 
AddImage(image_pack* Packer, const char* Filename) {
    image_entry* Entry = Packer->Entries +  Packer->EntryCount;
    Entry->Filename = Filename;
    stbi_info(Filename, &Entry->W, &Entry->H, 0);
    printf("Added %s: Width = %d, Height = %d\n", Filename, Entry->W, Entry->H);
    
    ++Packer->EntryCount;
}

static inline b32 
Pack(image_pack* Packer, u32 AtlasWidth, u32 AtlasHeight) {
    rect Spaces[1024];
    u32 SpaceCount = 0;
    
    Spaces->X = 0;
    Spaces->Y = 0;
    Spaces->W = AtlasWidth;
    Spaces->H = AtlasHeight;
    ++SpaceCount;
    
    printf("=== Packing...\n");
    {
        for ( u32 i = 0; i < Packer->EntryCount; ++i ) {
            image_entry* Entry = Packer->Entries + i;
            printf("\tPacking %s...\n", Entry->Filename);
            
            // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
            u32 ChosenSpaceIndex = SpaceCount;
            {
                for (u32 j = SpaceCount - 1; j >= 0; --j ) {
                    rect* Space = Spaces + j;
                    // NOTE(Momo): Check if the image fits
                    if (Entry->W <= Space->W && Entry->H <= Space->H) {
                        ChosenSpaceIndex = j;
                        
                        break;
                        
                    }
                }
            }
            
            
            // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
            if (ChosenSpaceIndex == SpaceCount) {
                printf("\tFit not found, terminating\n");
                return false;
            }
            
            // NOTE(Momo): Swap and pop the chosen space
            rect ChosenSpace = Spaces[ChosenSpaceIndex];
            //printf("Chosen space %d: %d %d %d %d\n",ChosenSpaceIndex, ChosenSpace.X, ChosenSpace.Y, ChosenSpace.W, ChosenSpace.H );
            if (SpaceCount > 0) {
                Spaces[ChosenSpaceIndex] = Spaces[SpaceCount - 1];
                --SpaceCount;
            }
            
            // NOTE(Momo): Split if not perfect fit
            if (ChosenSpace.W != Entry->W && ChosenSpace.H == Entry->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Entry->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Entry->W;
                SplitSpaceRight.H = ChosenSpace.H;
                Spaces[SpaceCount++] = SplitSpaceRight;
                
            }
            else if (ChosenSpace.W == Entry->W && ChosenSpace.H != Entry->H) {
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Entry->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Entry->H;
                Spaces[SpaceCount++] = SplitSpaceDown;
                
            }
            else if (ChosenSpace.W != Entry->W && ChosenSpace.H != Entry->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Entry->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Entry->W;
                SplitSpaceRight.H = Entry->H;
                
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Entry->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Entry->H;
                
                // Choose to insert the bigger one first before the smaller one
                u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
                u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
                if (RightArea > DownArea) {
                    Spaces[SpaceCount++] = SplitSpaceRight;
                    Spaces[SpaceCount++] = SplitSpaceDown;
                }
                else {
                    Spaces[SpaceCount++] = SplitSpaceDown;
                    Spaces[SpaceCount++] = SplitSpaceRight;
                }
                
            }
            
            // NOTE(Momo): Add to packed images
            Entry->X = ChosenSpace.X;
            Entry->Y = ChosenSpace.Y;
        }
    }
    
    // Debug purposes
#if 1
    printf("--- Results:\n");
    for ( u32 i = 0; i < Packer->EntryCount; ++i ) {
        auto* Entry = Packer->Entries + i;
        printf("\tPacked %s at: X = %d, Y = %d, W = %d, H = %d\n", Entry->Filename, Entry->X, Entry->Y, Entry->W, Entry->H);
    }
#endif
    printf("=== Pack completed!\n");
    return true;
}


static inline void
OutputToPng(image_pack* Pack, u32 AtlasWidth, u32 AtlasHeight, u32 AtlasChannels, const char* Filename) 
{
    u32 MemorySize = AtlasWidth * AtlasHeight * AtlasChannels;
    u8* Memory = (u8*)calloc(MemorySize, sizeof(u8));
    if (!Memory) {
        printf("Failed to get memory\n");
        return;
    }
    Defer { free(Memory); };
    
    
    printf("=== Writing to %s\n", Filename);
    for (u32 i = 0; i < Pack->EntryCount; ++i) {
        auto* Entry = Pack->Entries + i;
        printf("\tLoading %s\n", Entry->Filename);
        
        
        i32 W,H,C;
        u8* LoadedImage = stbi_load(Entry->Filename, &W, &H, &C, 0);
        if (LoadedImage == nullptr) {
            printf("Failed to load :(\n");
            return;
        }
        Defer { stbi_image_free(LoadedImage); };
        Assert(W == Entry->W && H == Entry->H);
        
        
        
        i32 j = 0;
        for (i32 y = Entry->Y; y < Entry->Y + Entry->H; ++y) {
            for (i32 x = Entry->X; x < Entry->X + Entry->W; ++x) {
                i32 Index = TwoToOne(y, x, AtlasWidth) * AtlasChannels;
                for (u32 c = 0; c < AtlasChannels; ++c) {
                    Memory[Index + c] = LoadedImage[j++];
                }
            }
        }
        
    }
    
    stbi_write_png(Filename, AtlasWidth, AtlasHeight, AtlasChannels, Memory, AtlasWidth * AtlasChannels);
    printf("=== Write to %s completed\n", Filename);
}


int main() {
    image_pack Packer_ = {};
    image_pack* Packer = &Packer_;
    
    
    AddImage(Packer, "assets/ryoji.png");
    AddImage(Packer, "assets/yuu.png");
    AddImage(Packer, "assets/karu.png");
    
    // TODO(Momo): Might wanna sort first?
    // Sort(Packer);
    u32 AtlasWidth = 1024;
    u32 AtlasHeight = 1024;
    
    Pack(Packer, AtlasWidth, AtlasHeight);
    
    OutputToPng(Packer, AtlasWidth, AtlasHeight, 4, "debug.png" );
    
    
    
}