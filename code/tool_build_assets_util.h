/* date = March 14th 2021 2:39 pm */

#ifndef TOOL_BUILD_ASSETS_UTIL_H
#define TOOL_BUILD_ASSETS_UTIL_H

struct Loaded_Font {
	stbtt_fontinfo info;
    void* data;


};

struct Read_File_Result {
    void* data;
    u32 size;
};


static inline b8
read_file_into_memory(Read_File_Result* result,
                       Arena* arena, 
                       const char* filename) {
    
    FILE* File = nullptr;
    fopen_s(&File, filename, "rb");
    
    if (File == nullptr) {
        return false;
    }
    defer { fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    u32 size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* Buffer = arena->push_block(size);
    fread(Buffer, size, 1, File);
    
    result->data = Buffer;
    result->size = size;
    
    return true;
}

static inline b8
load_font(Loaded_Font* ret, Arena* arena, const char* filename) {
    Read_File_Result result = {};
    if (!read_file_into_memory(&result,
                                arena, 
                                filename)) 
    {
        return false;
    }
    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8*)result.data, 0);
    
    ret->info = Font;
    ret->data = result.data;
    
    return true;
}

#endif //TOOL_BUILD_ASSETS_UTIL_H
