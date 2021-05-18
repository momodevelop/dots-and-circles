#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct stream {
    u8* Contents;
    u32 ContentSize;
    
    u32 Current;
    
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

static inline void
Stream_GotoBeginning(stream* S) {
    S->BitBuffer = 0;
    S->BitCount = 0;
    S->Current = 0;
}

static inline b32
Stream_IsEos(stream* S) {
    return S->Current >= S->ContentSize;
}

static inline void*
Stream_ConsumeBlock(stream* S, u32 Amount) {
    void* Ret = nullptr;
    if (S->Current + Amount <= S->ContentSize) {
        Ret = S->Contents + S->Current;
    }
    S->Current += Amount;
    return Ret;
}

template<typename type>
static inline type*
Stream_Consume(stream* S) {
    return (type*)Stream_ConsumeBlock(S, sizeof(type));
}

// Bits are consumed from LSB to MSB
static inline u32
Stream_ConsumeBits(stream* S, u32 Amount){
    Assert(Amount <= 32);
    
    while(S->BitCount < Amount) {
        u32 Byte = *Stream_Consume<u8>(S);
        S->BitBuffer |= (Byte << S->BitCount);
        S->BitCount += 8;
    }
    
    u32 Result = S->BitBuffer & ((1 << Amount) - 1); 
    
    S->BitCount -= Amount;
    S->BitBuffer >>= Amount;
    
    return Result;
}

static inline b8
Stream_WriteBlock(stream* S, void* Src, u32 SrcSize) {
    if (S->Current + SrcSize >= S->ContentSize) {
        return false;
    }
    CopyBlock(S->Contents + S->Current, Src, SrcSize);
    S->Current += SrcSize; 
    return true;
}

template<typename type>
static inline b8
Stream_Write(stream* S, type Struct) {
    return Stream_WriteBlock(S, &Struct, sizeof(type));
}




#endif
