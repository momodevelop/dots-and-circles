#ifndef __MM_STREAM__
#define __MM_STREAM__

// TODO: Linked list of stream chunks?
struct stream {
    u8* Contents;
    u32 ContentSize;
    
    u32 Current;
};


static inline b8
Stream_Create(stream* Stream, void* Memory, u32 MemorySize) {
    if ( Memory == Null || MemorySize == 0) {
        return false;
    }
    Stream->Contents = (u8*)Memory;
    Stream->ContentSize = MemorySize;
    return true;
}



static inline b8
Stream_CreateFromArena(stream* Stream, MM_Arena* Arena, u32 Capacity) {
    void* Memory = MM_Arena_PushBlock(Arena, Capacity);
    return Stream_Create(Stream, Memory, Capacity); 
} 

static inline void
Stream_Reset(stream* S) {
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


//~ NOTE(Momo): Bitstream

struct bitstream {
    u8* Contents;
    u32 ContentSize;
    
    u32 Current;
    
    // For bit reading
    u32 BitBuffer;
    u32 BitCount;
};

static inline b8
Bitstream_Create(bitstream* Stream, void* Memory, u32 MemorySize) {
    if ( Memory == Null || MemorySize == 0) {
        return false;
    }
    Stream->Contents = (u8*)Memory;
    Stream->ContentSize = MemorySize;
    return true;
}



static inline b8
Bitstream_CreateFromArena(bitstream* Stream, MM_Arena* Arena, u32 Capacity) {
    void* Memory = MM_Arena_PushBlock(Arena, Capacity);
    return Bitstream_Create(Stream, Memory, Capacity); 
} 

static inline void
Bitstream_Reset(bitstream* S) {
    S->Current = 0;
}

static inline b32
Bitstream_IsEos(bitstream* S) {
    return S->Current >= S->ContentSize;
}

static inline void*
Bitstream_ConsumeBlock(bitstream* S, u32 Amount) {
    void* Ret = nullptr;
    if (S->Current + Amount <= S->ContentSize) {
        Ret = S->Contents + S->Current;
    }
    S->Current += Amount;
    return Ret;
}

template<typename type>
static inline type*
Bitstream_Consume(bitstream* S) {
    return (type*)Bitstream_ConsumeBlock(S, sizeof(type));
}


static inline b8
Bitstream_WriteBlock(bitstream* S, void* Src, u32 SrcSize) {
    if (S->Current + SrcSize >= S->ContentSize) {
        return false;
    }
    CopyBlock(S->Contents + S->Current, Src, SrcSize);
    S->Current += SrcSize; 
    return true;
}

template<typename type>
static inline b8
Bitstream_Write(bitstream* S, type Struct) {
    return Bitstream_WriteBlock(S, &Struct, sizeof(type));
}

// Bits are consumed from LSB to MSB
static inline u32
Bitstream_ConsumeBits(bitstream* S, u32 Amount){
    Assert(Amount <= 32);
    
    while(S->BitCount < Amount) {
        u32 Byte = *Bitstream_Consume<u8>(S);
        S->BitBuffer |= (Byte << S->BitCount);
        S->BitCount += 8;
    }
    
    u32 Result = S->BitBuffer & ((1 << Amount) - 1); 
    
    S->BitCount -= Amount;
    S->BitBuffer >>= Amount;
    
    return Result;
}


#endif
