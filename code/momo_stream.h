#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct Stream {
    u8* contents;
    u32 content_size;
    u32 current;
    
    inline b8 init(void* memory, u32 memory_size);
    inline b8 alloc(Arena* arena, u32 capacity);
    inline void reset();
    inline b8 is_eos();
    inline void* consume_block(u32 amount);
    
    template<typename T>
        inline T* consume_struct();
    
    inline b8 write_block(void* src, u32 src_size);
    template<typename T>
        inline b8 write_struct(T item);
};


b8
Stream::init(void* memory, u32 memory_size) {
    if ( memory == nullptr || memory_size == 0) {
        return false;
    }
    contents = (u8*)memory;
    content_size = memory_size;
    return true;
}



b8
Stream::alloc(Arena* arena, u32 capacity) {
    void* memory = arena->push_block(capacity);
    return init(memory, capacity); 
} 

void
Stream::reset() {
    current = 0;
}

b8
Stream::is_eos() {
    return current >= content_size;
}

void*
Stream::consume_block(u32 amount) {
    void* ret = nullptr;
    if (current + amount <= content_size) {
        ret = contents + current;
    }
    current += amount;
    return ret;
}

template<typename T>
T*
Stream::consume_struct() {
    return (T*)consume_block(sizeof(T));
}


b8
Stream::write_block(void* src, u32 src_size) {
    if (current + src_size >= content_size) {
        return false;
    }
    copy_block(contents + current, src, src_size);
    current += src_size; 
    return true;
}

template<typename T>
b8
Stream::write_struct(T item) {
    return write_block(&item, sizeof(T));
}


//~ NOTE(Momo): Bitstream

struct Bitstream {
    u8* contents;
    u32 content_size;
    u32 current;
    
    // For bit reading
    u32 bit_buffer;
    u32 bit_count;
    
    b8 init(void* memory, u32 memory_size);
    b8 alloc(Arena* arena, u32 capacity);
    void reset();
    b8 is_eos();
    void* consume_block(u32 amount);
    
    template<typename T>
        T* consume_struct();
    
    b8 write_block(void* src, u32 src_size);
    template<typename T>
        b8 write_struct(T item);
    
    u32 consume_bits(u32 amount);
    
};

b8
Bitstream::init(void* memory, u32 memory_size) {
    if ( memory == nullptr || memory_size == 0) {
        return false;
    }
    contents = (u8*)memory;
    content_size = memory_size;
    return true;
}



b8
Bitstream::alloc(Arena* arena, u32 capacity) {
    void* memory = arena->push_block(capacity);
    return init(memory, capacity); 
} 

void
Bitstream::reset() {
    current = 0;
}

b8
Bitstream::is_eos() {
    return current >= content_size;
}

void*
Bitstream::consume_block(u32 amount) {
    void* ret = nullptr;
    if (current + amount <= content_size) {
        ret = contents + current;
    }
    current += amount;
    return ret;
}

template<typename T>
T*
Bitstream::consume_struct() {
    return (T*)consume_block(sizeof(T));
}


b8
Bitstream::write_block(void* src, u32 src_size) {
    if (current + src_size >= content_size) {
        return false;
    }
    copy_block(contents + current, src, src_size);
    current += src_size; 
    return true;
}

template<typename T>
b8
Bitstream::write_struct(T item) {
    return write_block(&item, sizeof(T));
}

// Bits are consumed from LSB to MSB
u32
Bitstream::consume_bits(u32 amount){
    ASSERT(amount <= 32);
    
    while(bit_count < amount) {
        u32 byte = *consume_struct<u8>();
        bit_buffer |= (byte << bit_count);
        bit_count += 8;
    }
    
    u32 result = bit_buffer & ((1 << amount) - 1); 
    
    bit_count -= amount;
    bit_buffer >>= amount;
    
    return result;
}


#endif
