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
    
    static inline String create(const char* cstr);
};

union String_Buffer {
    String str;
    struct {
        u8* data;
        u32 count;
    };
    u32 cap;
};


//~ NOTE(Momo): Strings
static inline b8
init(String* s, u8* buffer, u32 buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    s->data = buffer;
    s->count = buffer_size;
    
    return true;
}

static inline b8
init(String* s, const char* cstr) {
    return init(s, (u8*)cstr, SiStrLen(cstr));
}


static inline b8
init_from_range(String* dest, String src, u32 min, u32 max) {
    if (min <= max) {
        return false;
    }
    return init(dest, src.data + min, max - min);
}

// Assumes C-String
static inline String
create_string(const char* cstr) {
    String ret = {};
    ret.data = (u8*)cstr;
    ret.count = SiStrLen(cstr);
    return ret;
    
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

static inline u32
find(String s, u8 item, u32 start_index) {
    for(u32 i = start_index; i < s.count; ++i) {
        if(s.data[i] == item) {
            return i;
        }
    }
    return s.count;
}

static inline String_Split_Result
split(String s, Arena* arena, u8 delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. Strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<String> struct.
    String_Split_Result ret = {};
    u32 min = 0;
    u32 max = 0;
    
    for (;max != s.count;) {
        max = find(s, delimiter, min);
        
        String* link = arena->push_struct<String>();
        // TODO: don't assert?
        Assert(link);
        init_from_range(link, s, min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): String_Buffer
static inline b8
init(String_Buffer* sb, u8* buffer, u32 capacity) {
    if (!buffer || capacity == 0) {
        return false;
    }
    sb->data = buffer;
    sb->count = 0;
    sb->cap = capacity;
    return true;
}


static inline b8
alloc(String_Buffer* sb, Arena* arena, u32 capacity) {
    u8* buffer = arena->push_array<u8>(capacity);
    return init(sb, buffer, capacity);
}

static inline b8
pop(String_Buffer* sb) {
    if (sb->count <= 0) {
        return false;
    }
    --sb->count;
    return true;
}

static inline u32 
remaining(String_Buffer sb) {
    return sb.cap - sb.count;
}

static inline b8
copy(String_Buffer* sb, String src) {
    if (src.count > sb->cap) {
        return false;
    }
    for (u32 I = 0; I < src.count; ++I ) {
        sb->data[I] = src.data[I];
    }
    sb->count = src.count;
    return true;
}


static inline b8
push(String_Buffer* sb, u8 item) {
    if (sb->count < sb->cap) {
        sb->data[sb->count++] = item;
        return true;
    }
    return false;
}

static inline b8
push(String_Buffer* sb, String src) {
    if (sb->count + src.count <= sb->cap) {
        for ( u32 I = 0; I < src.count; ++I ) {
            sb->data[sb->count++] = src.data[I];
        }
        return true;
    }
    return false;
}

static inline b8
push(String_Buffer* sb, String_Buffer src) {
    return push(sb, src.str);
}

static inline void 
clear(String_Buffer* sb) {
    sb->count = 0;
}

static inline b8
push(String_Buffer* sb, u32 num) {
    if (num == 0) {
        push(sb, (u8)'0');
        return true;
    }
    u32 start_pt = sb->count; 
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        b8 success = push(sb, (u8)(digit_to_convert + '0'));
        if (!success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (sb->count - start_pt)/2;
    for(u32 I = 0; I < sub_str_len_half; ++I) {
        Swap(u8, 
             sb->data[start_pt + I], 
             sb->data[sb->count - 1 - I]);
    }
    return true;
}


b8
push(String_Buffer* sb, s32 num) {
    if (num == 0) {
        if(!push(sb, '0')) {
            return false;
        }
        return true;
    }
    
    u32 start_pt = sb->count; 
    
    b8 Negative = num < 0;
    num = AbsOf(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        push(sb, (u8)(digit_to_convert + '0'));
    }
    
    if (Negative) {
        push(sb, (u8)'-');
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (sb->count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        Swap(u8, sb->data[start_pt + i], 
             sb->data[sb->count-1-i]);
        
    }
    
    return true;
}


#endif
