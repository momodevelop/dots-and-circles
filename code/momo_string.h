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
    
    b8 init(u8* buffer, u32 buffer_size);
    b8 init(const char* cstr);
    b8 init(String src, u32 min, u32 max);
    b8 is_equal(String rhs);
    b8 is_equal(const char* rhs);
    u32 find(u8 item, u32 start_index);
    String_Split_Result split(Arena* arena, u8 delimiter);
    
    b8 operator==(String rhs);
    b8 operator==(const c8* rhs);
    b8 operator!=(String rhs);
    b8 operator!=(const c8* rhs);
    
    static inline String create(const c8* cstr);
};

struct String_Buffer {
    union {
        String str;
        struct {
            u8* data;
            u32 count;
        };
    };
    u32 capacity;
    
    
    b8 init(u8* buffer, u32 buffer_size);
    b8 alloc(Arena* arena, u32 capacity);
    b8 pop();
    u32 remaining();
    b8 copy(String src);
    b8 push(u8);
    b8 push(String);
    b8 push(String_Buffer);
    b8 push(u32);
    b8 push(s32);
    void clear();
    
};

#include "momo_string.cpp"

#endif
