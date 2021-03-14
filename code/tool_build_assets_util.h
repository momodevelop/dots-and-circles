/* date = March 14th 2021 2:39 pm */

#ifndef TOOL_BUILD_ASSETS_UTIL_H
#define TOOL_BUILD_ASSETS_UTIL_H

struct loaded_font {
	stbtt_fontinfo Info;
    void* Data;
};

struct read_file_result {
    void* Data;
    u32 Size;
};


static inline aabb2u
Tab_PackerAabbToAabb2u(aabb_packer_aabb Aabb) {
	return { 
        Aabb.X, 
        Aabb.Y, 
        Aabb.X + Aabb.W, 
        Aabb.Y + Aabb.H 
    };    
}

static inline read_file_result
Tab_ReadFileIntoMemory(arena* Arena, 
                        const char* Filename) {
    read_file_result Ret = {};
    FILE* File = Null;
    fopen_s(&File, Filename, "rb");
    
    if (File == Null) {
        return Ret;
    }
    Defer { fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    u32 Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* Buffer = Arena_PushBlock(Arena, Size);
    fread(Buffer, Size, 1, File);
    
    Ret.Data = Buffer;
    Ret.Size = Size;
    
    return Ret;
}

static inline b32
Tab_LoadFont(loaded_font* Ret, arena* Arena, const char* Filename) {
    read_file_result ReadFileResult = Tab_ReadFileIntoMemory(Arena, Filename);
	if (!ReadFileResult.Data) {
        return FALSE;
    }
    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8*)ReadFileResult.Data, 0);
    
    Ret->Info = Font;
    Ret->Data = ReadFileResult.Data;
    
    return TRUE;
}

#endif //TOOL_BUILD_ASSETS_UTIL_H
