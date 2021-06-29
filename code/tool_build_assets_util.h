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


static inline b8
Tba_ReadFileIntoMemory(read_file_result* Result,
                       Arena* arena, 
                       const char* Filename) {
    
    FILE* File = nullptr;
    fopen_s(&File, Filename, "rb");
    
    if (File == nullptr) {
        return false;
    }
    defer { fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    u32 Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* Buffer = arena->push_block(Size);
    fread(Buffer, Size, 1, File);
    
    Result->Data = Buffer;
    Result->Size = Size;
    
    return true;
}

static inline b8
Tba_LoadFont(loaded_font* Ret, Arena* arena, const char* Filename) {
    read_file_result ReadFileResult = {};
    if (!Tba_ReadFileIntoMemory(&ReadFileResult,
                                arena, 
                                Filename)) 
    {
        return false;
    }
    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8*)ReadFileResult.Data, 0);
    
    Ret->Info = Font;
    Ret->Data = ReadFileResult.Data;
    
    return true;
}

#endif //TOOL_BUILD_ASSETS_UTIL_H
