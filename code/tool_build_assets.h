#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define MAX_ENTRIES 1024 

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


#include "game_asset_types.h"


// NOTE(Momo): File Structures  //////////////////////////////////////////////////////////////
#pragma pack(push, 1)


struct asset_file_entry {
    asset_type Type;
    u32 OffsetToData;
    asset_id Id;
};

struct asset_file_data_image {
    u32 Width;
    u32 Height;
    u32 Channels;
};

struct asset_file_data_font {
    u32 Width;
    u32 Height; 
    u32 Channels;
    u32 CharacterCount;
    // scale?
};

struct asset_file_data_font_character {
    u32 X, Y, W, H;
    u32 Codepoint;
};

struct asset_file_data_spritesheet {
    u32 Width;
    u32 Height;
    u32 Channels;
    
    u32 Rows;
    u32 Cols;
};

#pragma pack(pop)



// NOTE(Momo): Asset Builder //////////////////////////////////////////////////////////
struct asset_builder_entry_image {
    const char* Filename;
};

struct asset_builder_entry_sound {
    // TODO(Momo): 
};

struct asset_builder_entry_font {
    const char* Filename;
    const char* Characters;
    f32 Size;
};

struct asset_builder_entry_spritesheet {
    const char* Filename;
    u32 Rows;
    u32 Cols;
};

struct asset_builder_entry  {
    asset_type Type;
    asset_id Id;
    
    union {
        asset_builder_entry_image Image;
        asset_builder_entry_sound Sound;
        asset_builder_entry_spritesheet Spritesheet;
        asset_builder_entry_font Font;
    };
    
};


struct asset_builder {
    asset_builder_entry Entries[MAX_ENTRIES];
    u32 EntryCount;
};

#include "tool_build_assets.cc"

#endif //TOOL_BUILD_ASSETS_H
