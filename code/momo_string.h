#ifndef MM_STRING
#define MM_STRING


//~ NOTE(Momo): u8_cstr
// TODO(Momo): Support for UTF8??
struct u8_cstr {
    u8* data;
    u32 count;
    
    auto& operator[](u32 index) {
        Assert(index < count);
        return data[index];
    }
};

static inline b8
MM_U8CStr_Init(u8_cstr* s, u8* data, u32 count) {
    if (!data || count == 0) {
        return false;
    }
    s->data = data;
    s->count = count;
    
    return true;
}

// Assumes C-String
static inline b8
U8CStr_InitFromSiStr(u8_cstr* s, const char* si_str) {
    return MM_U8CStr_Init(s, (u8*)si_str, SiStrLen(si_str));
}

static inline b8
MM_U8CStr_Cmp(u8_cstr lhs, u8_cstr rhs) {
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
MM_U8CStr_CmpSiStr(u8_cstr lhs, const char* rhs) {
    for(u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static inline void
MM_U8CStr_SubString(u8_cstr* dest, u8_cstr src, u32 min, u32 max) {
    Assert(min <= max); 
    MM_U8CStr_Init(dest, src.data + min, max - min);
}

struct u8_cstr_split_res {
    u8_cstr* items;
    u32 item_count;
};

static inline u32
MM_U8CStr_Find(u8_cstr s, u8 item, u32 start_index) {
    for(u32 i = start_index; i < s.count; ++i) {
        if(s.data[i] == item) {
            return i;
        }
    }
    return s.count;
}

static inline u8_cstr_split_res
MM_U8CStr_SplitByDelimiter(u8_cstr s, MM_Arena* arena, u8 delimiter) {
    // NOTE(Momo): We are having faith that the MM_Arena given is a bump arena.
    // i.e. strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<string> struct.
    u8_cstr_split_res ret = {};
    u32 min = 0;
    u32 max = 0;
    
    for (;max != s.count;) {
        max = MM_U8CStr_Find(s, delimiter, min);
        
        u8_cstr* link = MM_Arena_PushStruct(u8_cstr, arena);
        Assert(link);
        MM_U8CStr_SubString(link, s, min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): u8_str
struct u8_str {
    union {
        u8_cstr cstr;
        struct {
            u8* data;
            u32 count;
        };
    };
    u32 cap;
    
    auto& operator[](u32 index) {
        Assert(index < count);
        return data[index];
    }
};

static inline b8
MM_U8Str_Init(u8_str* s, u8* buffer, u32 capacity) {
    if (!buffer || capacity == 0) {
        return false;
    }
    s->data = buffer;
    s->count = 0;
    s->cap = capacity;
    
    return true;
}


static inline b8
MM_U8Str_InitFromArena(u8_str* s, MM_Arena* arena, u32 capacity) {
    u8* buffer = MM_Arena_PushArray(u8, arena, capacity);
    if(!buffer) {
        return false;
    }
    s->data = buffer;
    s->count = 0;
    s->cap = capacity;
    
    return true;
}

static inline b8
MM_U8Str_Pop(u8_str* s) {
    if (s->count <= 0) {
        return false;
    }
    --s->count;
    return true;
}

static inline u32 
MM_U8Str_Remaining(u8_str* buffer) {
    return buffer->cap - buffer->count;
}

static inline b8
MM_U8Str_CopyCStr(u8_str* dest, u8_cstr src) {
    if (src.count > dest->cap) {
        return false;
    }
    for (u32 i = 0; i < src.count; ++i ) {
        dest->data[i] = src.data[i];
    }
    dest->count = src.count;
    return true;
}

static inline b8
MM_U8Str_Copy(u8_str* dest, u8_str* src) {
    return MM_U8Str_CopyCStr(dest, src->cstr);
}


static inline b8
MM_U8Str_NullTerm(u8_str* dest) {
    if (dest->count < dest->cap) {
        dest->data[dest->count] = 0;
        return true;
    }
    return false;
}

static inline b8
MM_U8Str_Push(u8_str* dest, u8 item) {
    if (dest->count < dest->cap) {
        dest->data[dest->count++] = item;
        return true;
    }
    return false;
}

static inline b8
MM_U8Str_PushCStr(u8_str* dest, u8_cstr src) {
    if (dest->count + src.count <= dest->cap) {
        for ( u32 i = 0; i < src.count; ++i ) {
            dest->data[dest->count++] = src.data[i];
        }
        return true;
    }
    return false;
}

static inline b8
MM_U8Str_PushStr(u8_str* dest, u8_str* src) {
    return MM_U8Str_PushCStr(dest, src->cstr);
}

static inline void 
MM_U8Str_Clear(u8_str* dest) {
    dest->count = 0;
}

static inline b8
MM_U8Str_PushU32(u8_str* dest, u32 num) {
    if (num == 0) {
        MM_U8Str_Push(dest, '0');
        return true;
    }
    u32 start_pt = dest->count; 
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        b8 success = MM_U8Str_Push(dest, (u8)(digit_to_convert + '0'));
        if (!success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 substr_len_halved = (dest->count - start_pt)/2;
    for(u32 I = 0; I < substr_len_halved; ++I) {
        Swap(u8, 
             dest->data[start_pt + I], 
             dest->data[dest->count - 1 - I]);
    }
    return true;
}

static inline b8
MM_U8Str_PushS32(u8_str* dest, s32 num) {
    if (num == 0) {
        if(!MM_U8Str_Push(dest, '0')) {
            return false;
        }
        return true;
    }
    
    u32 start_pt = dest->count; 
    
    b8 negative = num < 0;
    num = AbsOf(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        MM_U8Str_Push(dest, (char)(digit_to_convert + '0'));
    }
    
    if (negative) {
        MM_U8Str_Push(dest, '-');
    }
    
    // Reverse starting from start point to count
    u32 substr_len_halved = (dest->count - start_pt)/2;
    for(u32 i = 0; i < substr_len_halved; ++i) {
        Swap(u8, dest->data[start_pt + i], 
             dest->data[dest->count-1-i]);
        
    }
}


#endif
