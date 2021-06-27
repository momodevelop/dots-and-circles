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
    
    b8 init(u8* data, u32 count);
    b8 init_from_cstr(const char* cstr);
    b8 init_from_range(String src, u32 min, u32 max);
    u32 find(u8 item, u32 start_index = 0);
    String_Split_Result split(Arena* arena, u8 delimiter);
    
    static inline String create(const char* cstr);
};

//~ NOTE(Momo): String
b8
String::init(u8* buffer, u32 buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    data = buffer;
    count = buffer_size;
    
    return true;
}

b8
String::init_from_range(String src, u32 min, u32 max) {
    if (min <= max) {
        return false;
    }
    return init(src.data + min, max - min);
}

// Assumes C-String
inline String
String::create(const char* cstr) {
    String ret = {};
    ret.data = (u8*)cstr;
    ret.count = SiStrLen(cstr);
    return ret;
    
}

b8
String::init_from_cstr(const char* cstr) {
    return init((u8*)cstr, SiStrLen(cstr));
}

static inline b8
is_equal(String lhs, String rhs) {
    if(lhs.count != rhs.count) {
        return false;
    }
    for (u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}


static inline b8
is_equal(String lhs, const char* rhs) {
    for(u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

u32
String::find(u8 item, u32 start_index) {
    for(u32 i = start_index; i < count; ++i) {
        if(data[i] == item) {
            return i;
        }
    }
    return count;
}

String_Split_Result
String::split(Arena* arena, u8 delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. Strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<String> struct.
    String_Split_Result ret = {};
    u32 min = 0;
    u32 max = 0;
    
    for (;max != count;) {
        max = find(delimiter, min);
        
        String* link = arena->push_struct<String>();
        // TODO: don't assert?
        Assert(link);
        link->init_from_range((*this), min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): u8_str
union String_Buffer {
    String str;
    struct {
        u8* data;
        u32 count;
    };
    u32 cap;
    
    b8 init(u8* buffer, u32 capacity);
    b8 alloc(Arena* arena, u32 capacity);
    b8 pop();
    u32 remaining();
    b8 copy(String str);
    b8 push(u8 item);
    //b8 push(const char* cstr);
    b8 push(String str);
    b8 push(String_Buffer strbuf);
    b8 push(u32 num);
    b8 push(s32 num);
    void clear();
    
};


b8
String_Buffer::init(u8* buffer, u32 capacity) {
    if (!buffer || capacity == 0) {
        return false;
    }
    data = buffer;
    count = 0;
    cap = capacity;
    return true;
}


b8
String_Buffer::alloc(Arena* arena, u32 capacity) {
    u8* buffer = arena->push_array<u8>(capacity);
    return init(buffer, capacity);
}

b8
String_Buffer::pop() {
    if (count <= 0) {
        return false;
    }
    --count;
    return true;
}

u32 
String_Buffer::remaining() {
    return cap - count;
}

b8
String_Buffer::copy(String src) {
    if (src.count > cap) {
        return false;
    }
    for (u32 I = 0; I < src.count; ++I ) {
        data[I] = src.data[I];
    }
    count = src.count;
    return true;
}


b8
String_Buffer::push(u8 item) {
    if (count < cap) {
        data[count++] = item;
        return true;
    }
    return false;
}

b8
String_Buffer::push(String src) {
    if (count + src.count <= cap) {
        for ( u32 I = 0; I < src.count; ++I ) {
            data[count++] = src.data[I];
        }
        return true;
    }
    return false;
}

b8
String_Buffer::push(String_Buffer src) {
    return push(src.str);
}

void 
String_Buffer::clear() {
    count = 0;
}

b8
String_Buffer::push(u32 num) {
    if (num == 0) {
        push('0');
        return true;
    }
    u32 start_pt = count; 
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        b8 success = push((u8)(digit_to_convert + '0'));
        if (!success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (count - start_pt)/2;
    for(u32 I = 0; I < sub_str_len_half; ++I) {
        Swap(u8, 
             data[start_pt + I], 
             data[count - 1 - I]);
    }
    return true;
}


b8
String_Buffer::push(s32 num) {
    if (num == 0) {
        if(!push('0')) {
            return false;
        }
        return true;
    }
    
    u32 start_pt = count; 
    
    b8 Negative = num < 0;
    num = AbsOf(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        push((char)(digit_to_convert + '0'));
    }
    
    if (Negative) {
        push('-');
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        Swap(u8, data[start_pt + i], 
             data[count-1-i]);
        
    }
}


#endif
