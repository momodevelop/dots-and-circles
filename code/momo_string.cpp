

//~ NOTE(Momo): Strings
static inline String
String_Create(u8* buffer, u32 buffer_size) {
    String ret = {};
    ret.data = buffer;
    ret.count = buffer_size;
    
    return ret;
}


// Assumes C-String
static inline String
String_Create(const c8* cstr) {
    String ret = {};
    ret.data = (u8*)cstr;
    ret.count = cstr_length(cstr);
    return ret;
    
}

static inline String
String_Create(String src, u32 min, u32 max) {
    return String_Create(src.data + min, max - min);
}

static inline b8
String_IsEqual(String lhs, String rhs) {
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
String_IsEqual(String lhs, const char* rhs) {
    for(u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static inline b8 
operator==(String lhs, String rhs) {
    return String_IsEqual(lhs, rhs);
}

static inline b8 
operator!=(String lhs, String rhs) {
    return !String_IsEqual(lhs, rhs);
}


static inline b8 
operator==(String lhs, const char* rhs) {
    return String_IsEqual(lhs, rhs);
}

static inline b8 
operator!=(String lhs, const char* rhs) {
    return !String_IsEqual(lhs, rhs);
}

static inline u32
String_Find(String s, u8 item, u32 start_index = 0) {
    for(u32 i = start_index; i < s.count; ++i) {
        if(s.data[i] == item) {
            return i;
        }
    }
    return s.count;
}

//TODO: We should think of a non-arena way
String_Split_Result
String_Split(String s, Arena* arena, u8 delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. Strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<String> struct.
    String_Split_Result ret = {};
    u32 min = 0;
    u32 max = 0;
    
    for (;max != s.count;) {
        max = String_Find(s, delimiter, min);
        
        String* link = Arena_Push<String>(arena);
        // TODO: don't assert?
        ASSERT(link);
        (*link) = String_Create(s, min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): StringBuffer
b8
StringBuffer_Init(StringBuffer* s, u8* buffer, u32 buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    s->data = buffer;
    s->count = 0;
    s->capacity = buffer_size;
    return true;
}


b8
StringBuffer_Alloc(StringBuffer* s, Arena* arena, u32 size) {
    u8* buffer = Arena_PushArray<u8>(arena, size);
    return StringBuffer_Init(s, buffer, size);
}

b8
StringBuffer_Pop(StringBuffer* s) {
    if (s->count <= 0) {
        return false;
    }
    --s->count;
    return true;
}

u32
StringBuffer_Remaining(StringBuffer* s) {
    return s->capacity - s->count;
}

b8
StringBuffer_Copy(StringBuffer* s, String src) {
    if (src.count > s->capacity) {
        return false;
    }
    for (u32 i = 0; i < src.count; ++i ) {
        s->data[i] = src.data[i];
    }
    s->count = src.count;
    return true;
}




b8
StringBuffer_Push(StringBuffer* s, u8 item) {
    if (s->count < s->capacity) {
        s->data[s->count++] = item;
        return true;
    }
    return false;
}

b8
StringBuffer_Push(StringBuffer* s, String src) {
    if (s->count + src.count <= s->capacity) {
        for ( u32 i = 0; i < src.count; ++i ) {
            s->data[s->count++] = src.data[i];
        }
        return true;
    }
    return false;
}

b8
StringBuffer_Push(StringBuffer* s, StringBuffer src) {
    return StringBuffer_Push(s, src.str);
}

void 
StringBuffer_Clear(StringBuffer* s) {
    s->count = 0;
}

b8
StringBuffer_Push(StringBuffer* s, u32 num) {
    if (num == 0) {
        StringBuffer_Push(s, (u8)'0');
        return true;
    }
    u32 start_pt = s->count; 
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        b8 success = StringBuffer_Push(s, (u8)(digit_to_convert + '0'));
        if (!success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (s->count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        SWAP(s->data[start_pt + i], s->data[s->count - 1 - i]);
    }
    return true;
}


b8
StringBuffer_Push(StringBuffer* s, s32 num) {
    if (num == 0) {
        if(!StringBuffer_Push(s, '0')) {
            return false;
        }
        return true;
    }
    
    u32 start_pt = s->count; 
    
    b8 Negative = num < 0;
    num = ABS(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        StringBuffer_Push(s, (u8)(digit_to_convert + '0'));
    }
    
    if (Negative) {
        StringBuffer_Push(s, (u8)'-');
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (s->count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        SWAP(s->data[start_pt + i], 
             s->data[s->count-1-i]);
        
    }
    
    return true;
}
