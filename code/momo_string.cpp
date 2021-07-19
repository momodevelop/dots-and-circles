

//~ NOTE(Momo): Strings
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
String::init(const c8* cstr) {
    return init((u8*)cstr, cstr_length(cstr));
}


b8
String::init(String src, u32 min, u32 max) {
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
    ret.count = cstr_length(cstr);
    return ret;
    
}

b8
String::is_equal(String rhs) {
    if(count != rhs.count) {
        return false;
    }
    for (u32 i = 0; i < count; ++i) {
        if (data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}


b8
String::is_equal(const char* rhs) {
    for(u32 i = 0; i < count; ++i) {
        if (data[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

b8 
String::operator==(String rhs) {
    return is_equal(rhs);
}

b8 
String::operator!=(String rhs) {
    return !is_equal(rhs);
}


b8 
String::operator==(const char* rhs) {
    return is_equal(rhs);
}

b8 
String::operator!=(const char* rhs) {
    return !is_equal(rhs);
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
        
        String* link = Arena_Push<String>(arena);
        // TODO: don't assert?
        ASSERT(link);
        link->init((*this), min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): String_Buffer
b8
String_Buffer::init(u8* buffer, u32 buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    this->data = buffer;
    this->count = 0;
    this->capacity = buffer_size;
    return true;
}


b8
String_Buffer::alloc(Arena* arena, u32 size) {
    u8* buffer = Arena_PushArray<u8>(arena, size);
    return init(buffer, size);
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
    return capacity - count;
}

b8
String_Buffer::copy(String src) {
    if (src.count > capacity) {
        return false;
    }
    for (u32 i = 0; i < src.count; ++i ) {
        data[i] = src.data[i];
    }
    count = src.count;
    return true;
}




b8
String_Buffer::push(u8 item) {
    if (count < capacity) {
        data[count++] = item;
        return true;
    }
    return false;
}

b8
String_Buffer::push(String src) {
    if (count + src.count <= capacity) {
        for ( u32 i = 0; i < src.count; ++i ) {
            data[count++] = src.data[i];
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
        push((u8)'0');
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
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        SWAP(data[start_pt + i], data[count - 1 - i]);
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
    num = ABS(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        push((u8)(digit_to_convert + '0'));
    }
    
    if (Negative) {
        push((u8)'-');
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        SWAP(data[start_pt + i], 
             data[count-1-i]);
        
    }
    
    return true;
}
