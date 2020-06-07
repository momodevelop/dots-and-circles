#include <stdio.h>
#include "ryoji_common.cpp"



// NOTE(Momo): Allocator assumed to be void (*fp)(usize size, u8 alignment)
pure const char * FileToString(void (*allocator)(usize, u8), const char path) {
    FILE* file = fopen(path, "r");
    if (file == nullptr) {
        return file;
    }
    
    
    
    
}

// NOTE(Momo): Non-allocator version
pure const char * FileToString() {
}

