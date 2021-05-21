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


static inline b32
Tba_ReadFileIntoMemory(read_file_result* Result,
                       MM_Arena* Arena, 
                       const char* Filename) {
    
    FILE* File = Null;
    fopen_s(&File, Filename, "rb");
    
    if (File == Null) {
        return False;
    }
    Defer { fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    u32 Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* Buffer = MM_Arena_PushBlock(Arena, Size);
    fread(Buffer, Size, 1, File);
    
    Result->Data = Buffer;
    Result->Size = Size;
    
    return True;
}

static inline b32
Tba_LoadFont(loaded_font* Ret, MM_Arena* Arena, const char* Filename) {
    read_file_result ReadFileResult = {};
    if (!Tba_ReadFileIntoMemory(&ReadFileResult,
                                Arena, 
                                Filename)) 
    {
        return False;
    }
    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8*)ReadFileResult.Data, 0);
    
    Ret->Info = Font;
    Ret->Data = ReadFileResult.Data;
    
    return True;
}

#endif //TOOL_BUILD_ASSETS_UTIL_H
