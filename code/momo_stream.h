#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct Stream {
    u8* contents;
    u32 content_size;
    u32 current;
    
};


static inline b8
Stream_Init(Stream* s, void* memory, u32 memory_size) {
    if ( memory == nullptr || memory_size == 0) {
        return false;
    }
    s->contents = (u8*)memory;
    s->content_size = memory_size;
    return true;
}



static inline b8
Stream_Alloc(Stream* s, Arena* arena, u32 capacity) {
    void* memory = Arena_Push_Block(arena, capacity);
    return Stream_Init(s, memory, capacity); 
} 

static inline void
Stream_Reset(Stream* s) {
    s->current = 0;
}

static inline b8
Stream_Is_Eos(Stream* s) {
    return s->current >= s->content_size;
}

static inline void*
Stream_Consume_Block(Stream* s, u32 amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->content_size) {
        ret = s->contents + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename T>
static inline T*
Stream_Consume(Stream* s) {
    return (T*)Stream_Consume_Block(s, sizeof(T));
}


static inline b8
Stream_Write_Block(Stream* s, void* src, u32 src_size) {
    if (s->current + src_size >= s->content_size) {
        return false;
    }
    CopyBlock(s->contents + s->current, src, src_size);
    s->current += src_size; 
    return true;
}

template<typename T>
static inline b8
Stream_Write(Stream* s, T item) {
    return Stream_Write_Block(s, &item, sizeof(T));
}


//~ NOTE(Momo): Bitstream

struct Bitstream {
    u8* contents;
    u32 content_size;
    u32 current;
    
    // For bit reading
    u32 bit_buffer;
    u32 bit_count;
};

static inline b8
Bitstream_Init(Bitstream* s, void* memory, u32 memory_size) {
    if ( memory == nullptr || memory_size == 0) {
        return false;
    }
    s->contents = (u8*)memory;
    s->content_size = memory_size;
    return true;
}



static inline b8
Bitstream_Alloc(Bitstream* s, Arena* arena, u32 capacity) {
    void* memory = Arena_Push_Block(arena, capacity);
    return Bitstream_Init(s, memory, capacity); 
} 

static inline void
Bitstream_Reset(Bitstream* s) {
    s->current = 0;
}

static inline b8
Bitstream_IsEos(Bitstream* s) {
    return s->current >= s->content_size;
}

static inline void*
Bitstream_ConsumeBlock(Bitstream* s, u32 amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->content_size) {
        ret = s->contents + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename T>
static inline T*
Bitstream_Consume(Bitstream* s) {
    return (T*)Bitstream_ConsumeBlock(s, sizeof(T));
}


static inline b8
Bitstream_WriteBlock(Bitstream* s, void* src, u32 src_size) {
    if (s->current + src_size >= s->content_size) {
        return false;
    }
    CopyBlock(s->contents + s->current, src, src_size);
    s->current += src_size; 
    return true;
}

template<typename T>
static inline b8
Bitstream_Write(Bitstream* s, T item) {
    return Bitstream_WriteBlock(s, &item, sizeof(T));
}

// Bits are consumed from LSB to MSB
u32
Bitstream_ConsumeBits(Bitstream* s, u32 amount){
    ASSERT(amount <= 32);
    
    while(s->bit_count < amount) {
        u32 byte = *Bitstream_Consume<u8>(s);
        s->bit_buffer |= (byte << s->bit_count);
        s->bit_count += 8;
    }
    
    u32 result = s->bit_buffer & ((1 << amount) - 1); 
    
    s->bit_count -= amount;
    s->bit_buffer >>= amount;
    
    return result;
}


#endif
