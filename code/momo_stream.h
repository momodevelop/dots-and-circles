#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct MM_Stream {
    u8* contents;
    u32 content_size;
    
    u32 current;
};


static inline b8
MM_Stream_Create(MM_Stream* s, void* memory, u32 memory_size) {
    if ( memory == Null || memory_size == 0) {
        return false;
    }
    s->contents = (u8*)memory;
    s->content_size = memory_size;
    return true;
}



static inline b8
MM_Stream_New(MM_Stream* s, MM_Arena* arena, u32 capacity) {
    void* memory = MM_Arena_PushBlock(arena, capacity);
    return MM_Stream_Create(s, memory, capacity); 
} 

static inline void
MM_Stream_Reset(MM_Stream* s) {
    s->current = 0;
}

static inline b32
MM_Stream_IsEos(MM_Stream* s) {
    return s->current >= s->content_size;
}

static inline void*
MM_Stream_ConsumeBlock(MM_Stream* s, u32 amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->content_size) {
        ret = s->contents + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename T>
static inline T*
MM_Stream_Consume(MM_Stream* s) {
    return (T*)MM_Stream_ConsumeBlock(s, sizeof(T));
}


static inline b8
MM_Stream_WriteBlock(MM_Stream* s, void* Src, u32 src_size) {
    if (s->current + src_size >= s->content_size) {
        return false;
    }
    CopyBlock(s->contents + s->current, Src, src_size);
    s->current += src_size; 
    return true;
}

template<typename T>
static inline b8
MM_Stream_Write(MM_Stream* s, T item) {
    return MM_Stream_WriteBlock(s, &item, sizeof(T));
}


//~ NOTE(Momo): Bitstream

struct MM_Bitstream {
    u8* contents;
    u32 content_size;
    
    u32 current;
    
    // For bit reading
    u32 bit_buffer;
    u32 bit_count;
};

static inline b8
MM_Bitstream_Create(MM_Bitstream* s, void* memory, u32 memory_size) {
    if ( memory == Null || memory_size == 0) {
        return false;
    }
    s->contents = (u8*)memory;
    s->content_size = memory_size;
    return true;
}



static inline b8
MM_Bitstream_New(MM_Bitstream* s, MM_Arena* arena, u32 capacity) {
    void* memory = MM_Arena_PushBlock(arena, capacity);
    return MM_Bitstream_Create(s, memory, capacity); 
} 

static inline void
MM_Bitstream_Reset(MM_Bitstream* s) {
    s->current = 0;
}

static inline b32
MM_Bitstream_IsEos(MM_Bitstream* s) {
    return s->current >= s->content_size;
}

static inline void*
MM_Bitstream_ConsumeBlock(MM_Bitstream* s, u32 amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->content_size) {
        ret = s->contents + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename T>
static inline T*
MM_Bitstream_Consume(MM_Bitstream* s) {
    return (T*)MM_Bitstream_ConsumeBlock(s, sizeof(T));
}


static inline b8
MM_Bitstream_WriteBlock(MM_Bitstream* s, void* Src, u32 src_size) {
    if (s->current + src_size >= s->content_size) {
        return false;
    }
    CopyBlock(s->contents + s->current, Src, src_size);
    s->current += src_size; 
    return true;
}

template<typename T>
static inline b8
MM_Bitstream_Write(MM_Bitstream* s, T item) {
    return MM_Bitstream_WriteBlock(s, &item, sizeof(T));
}

// Bits are consumed from LSB to MSB
static inline u32
MM_Bitstream_ConsumeBits(MM_Bitstream* s, u32 amount){
    Assert(amount <= 32);
    
    while(s->bit_count < amount) {
        u32 Byte = *MM_Bitstream_Consume<u8>(s);
        s->bit_buffer |= (Byte << s->bit_count);
        s->bit_count += 8;
    }
    
    u32 ret = s->bit_buffer & ((1 << amount) - 1); 
    
    s->bit_count -= amount;
    s->bit_buffer >>= amount;
    
    return ret;
}


#endif
