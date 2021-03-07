#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct stream {
    array<u8> Contents;
    usize Current;
    
    // For bit reading
    u32 BitBuffer;
    u32 BitCount;
};

static inline stream
CreateStream(void* Memory, usize MemorySize) {
    stream Ret = {};
    Ret.Contents = Array_Create((u8*)Memory, MemorySize);
    return Ret;
}

static inline stream
CreateStream(arena* Arena, usize Capacity) {
    void* Memory = Arena_PushBlock(Arena, Capacity);
    return CreateStream(Memory, Capacity); 
} 

static inline b32
IsEos(stream* S) {
    return S->Current >= S->Contents.Count;
}

static inline void*
Consume(stream* S, usize Amount) {
    void* Ret = nullptr;
    if (S->Current + Amount <= S->Contents.Count) {
        Ret = S->Contents.Elements + S->Current;
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
Write(stream* S, void* Src, usize SrcSize) {
    if (S->Current + SrcSize >= S->Contents.Count) {
        return false;
    }
    Copy(S->Contents.Elements + S->Current, Src, SrcSize);
    S->Current += SrcSize; 
    return true;
}

template<typename t>
static inline b32
Write(stream* S, t Struct){
    return Write(S, &Struct, sizeof(t));
}

#endif
