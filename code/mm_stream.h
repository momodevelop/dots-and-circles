#ifndef __MM_STREAM__
#define __MM_STREAM__

#include "mm_array.h"

// TODO: Linked list of stream chunks?
struct Stream {
    Array<u8> contents;
    usize current;

    // For bit reading
    u32 bit_buffer;
    u32 bit_count;
};

static inline Stream
stream(void* memory, usize memory_size) {
    Stream Ret = {};
    Ret.contents = array((u8*)memory, memory_size);
    return Ret;
}

static inline Stream
stream(Memory_Arena* arena, usize capacity) {
    void* Memory = PushBlock(arena, capacity);
    return stream(Memory, capacity); 
} 

static inline b32
is_eos(Stream* s) {
    return s->current >= s->contents.count;
}

static inline void*
consume(Stream* s, usize amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->contents.count) {
        ret = s->contents.elements + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename t>
static inline t* 
consume(Stream* s) {
    return (t*)consume(s, sizeof(t));
}


// Bits are consumed from LSB to MSB
static inline u32
consume_bits(Stream* s, u32 amount)
{
    Assert(amount <= 32);
    
    while(s->bit_count < amount) {
        u32 byte = *consume<u8>(s);
        s->bit_buffer |= (byte << s->bit_count);
        s->bit_count += 8;
    }

    u32 Result = s->bit_buffer & ((1 << amount) - 1); 

    s->bit_count -= amount;
    s->bit_buffer >>= amount;

    return Result;
}

static inline b32
write(Stream* s, void* src, usize src_size) {
    if (s->current + src_size >= s->contents.count) {
        return false;
    }
    Copy(s->contents.elements + s->current, src, src_size);
    s->current += src_size; 
    return true;
}

template<typename t>
static inline b32
write(Stream* s, t Struct){
    return write(s, &Struct, sizeof(t));
}

#endif
