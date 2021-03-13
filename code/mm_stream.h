#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct stream {
    u8* Contents;
    u32 ContentSize;

    usize Current;
    
    // For bit reading
    u32 BitBuffer;
    u32 BitCount;
};

static inline stream
Stream_CreateFromMemory(void* Memory, u32 MemorySize) {
    stream Ret = {};
    Ret.Contents = (u8*)Memory;
    Ret.ContentSize = MemorySize;
    return Ret;
}

static inline stream
Stream_CreateFromArena(arena* Arena, u32 Capacity) {
    void* Memory = Arena_PushBlock(Arena, Capacity);
    return Stream_CreateFromMemory(Memory, Capacity); 
} 

static inline b32
IsEos(stream* S) {
    return S->Current >= S->ContentSize;
}

static inline void*
Consume(stream* S, usize Amount) {
    void* Ret = nullptr;
    if (S->Current + Amount <= S->ContentSize) {
        Ret = S->Contents + S->Current;
    }
    S->Current += Amount;
    return Ret;
}

template<typename t>
static inline t* 
Consume(stream* S) {
    return (t*)Consume(S, sizeof(t));
}

// Bits are consumed from LSB to MSB
static inline u32
ConsumeBits(stream* S, u32 Amount){
    Assert(Amount <= 32);
    
    while(S->BitCount < Amount) {
        u32 Byte = *Consume<u8>(S);
        S->BitBuffer |= (Byte << S->BitCount);
        S->BitCount += 8;
    }
    
    u32 Result = S->BitBuffer & ((1 << Amount) - 1); 
    
    S->BitCount -= Amount;
    S->BitBuffer >>= Amount;
    
    return Result;
}

static inline b32
Write(stream* S, void* Src, u32 SrcSize) {
    if (S->Current + SrcSize >= S->ContentSize) {
        return false;
    }
    Copy(S->Contents + S->Current, Src, SrcSize);
    S->Current += SrcSize; 
    return true;
}

template<typename t>
static inline b32
Write(stream* S, t Struct){
    return Write(S, &Struct, sizeof(t));
}

#endif
