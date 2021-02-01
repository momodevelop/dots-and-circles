#ifndef __MM_STREAM__
#define __MM_STREAM__

#include "mm_array.h"

// TODO: Linked list of stream chunks?
struct stream {
    array<u8> Contents;
    usize Current;

    // For bit reading
    u32 BitBuffer;
    u32 BitCount;
};

static inline stream
Stream(void* Memory, usize MemorySize) {
    stream Ret = {};
    Ret.Contents = Array((u8*)Memory, MemorySize);
    return Ret;
}

static inline b32
IsEos(stream* Stream) {
    return Stream->Current >= Stream->Contents.Count;
}


static inline void*
Peek(stream* Stream, usize Amount) {
    // Will reach end of stream
    if (Stream->Current + Amount > Stream->Contents.Count) {
        return nullptr;
    }
    return Stream->Contents.Elements + Stream->Current;
}

template<typename t>
static inline t*
Peek(stream* Stream) {
    return (t*)Peek(Stream, sizeof(t));
}

static inline void
Advance(stream* Stream, usize Amount) {
    Stream->Current += Amount;
}

template<typename t>
static inline void
Advance(stream* Stream) {
    Advance(Stream, sizeof(t));
}

static inline void*
Consume(stream* Stream, usize Amount) {
    void* Ret = Peek(Stream, Amount);
    Advance(Stream, Amount);
    return Ret;
}

template<typename t>
static inline t* 
Consume(stream* Stream) {
    return (t*)Consume(Stream, sizeof(t));
}

// TODO: Untested
static inline u32
ConsumeBits(stream* Stream, u32 Amount){
    Assert(Amount <= 32);
    
    while(Stream->BitCount < Amount) {
        u32 Byte = *Consume<u8>(Stream);
        Stream->BitBuffer <<= 8;
        Stream->BitBuffer |= Byte;
        Stream->BitCount += 8;
    }

    // TODO: There probably is a way to optimize this?
    // Can do that for exercise
    u32 LocationDistance = Stream->BitCount - Amount;
    u32 Flag = ((1 << Amount) - 1) << LocationDistance;
    u32 ResultAtLocation = (Stream->BitBuffer & Flag);
    u32 Result = ResultAtLocation >> LocationDistance; 

    Stream->BitCount -= Amount;
    Stream->BitBuffer &= ~(Stream->BitBuffer & Flag);

    return Result;
}


#endif
