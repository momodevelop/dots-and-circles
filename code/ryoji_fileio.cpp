#ifndef __RYOJI_FILEIO__
#define __RYOJI_FILEIO__

#include <cstdio>

#define FOREACH_ERRORS(ERR) \
ERR(ERR_FILEIO_NONE)   \
ERR(ERR_FILEIO_FILE_CANNOT_OPEN) \
ERR(ERR_FILEIO_DESTINATION_TOO_SMOL) \


GenerateEnumStrings(ErrFileIO, ErrFileIOStr, FOREACH_ERRORS)
#undef FOREACH_ERRORS

pure 
ErrFileIO 
ReadFileStr(char* dest, u64 destSize, const char * path) {
    FILE* file = fopen(path, "r");
    if (file == nullptr) {
        return ERR_FILEIO_FILE_CANNOT_OPEN;
    }
    Defer{
        fclose(file);
    };
    
    // Get file size
    fseek(file, 0, SEEK_END);
    u64 filesize = ftell(file); // Does not include EOF
    fseek(file, 0, SEEK_SET);
    
    if ((filesize + 1) > destSize) {
        return ERR_FILEIO_DESTINATION_TOO_SMOL;
    }
    
    fread(dest, sizeof(char), filesize, file);
    
    // Don't forget null terminating value
    dest[filesize] = 0;
    
    return ERR_FILEIO_NONE;
}

pure 
ErrFileIO
ReadFileBin(void* dest, u64 destSize, const char * path) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return ERR_FILEIO_FILE_CANNOT_OPEN;
    }
    Defer{
        fclose(file);
    };
    
    // Get file size
    fseek(file, 0, SEEK_END);
    u64 filesize = ftell(file); // Does not include EOF
    fseek(file, 0, SEEK_SET);
    
    if (filesize > destSize) {
        return ERR_FILEIO_DESTINATION_TOO_SMOL;
    }
    fread(dest, sizeof(char), filesize, file);
    return ERR_FILEIO_NONE;
}




#endif