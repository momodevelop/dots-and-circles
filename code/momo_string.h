#ifndef MM_STRING
#define MM_STRING


// TODO(Momo): Support for UTF8??

struct String;
struct String_Split_Result {
    String* items;
    u32 item_count;
};

struct String {
    u8* data;
    u32 count;
};

struct StringBuffer {
    union {
        String str;
        struct {
            u8* data;
            u32 count;
        };
    };
    u32 capacity;
    
    
};

#include "momo_string.cpp"

#endif
