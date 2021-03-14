#ifndef __MOMO_BITWISE__
#define __MOMO_BITWISE__

// Note that all alignment operations is catered to only power of 2!
// Reference: https://stackoverflow.com/questions/227897/how-to-alloc-aligned-memory-only-using-the-standard-library
// 
// Let addressToAlign = A, byteToAlign = N
// To align A to N-byte is to align A to a byte that is 2^N.
// e.g. To align 0011 to 4-byte -> the result should be 0100 = 4
// e.g. To align 0110 to 4-byte -> the result should be 1000 = 8
//
// To align, we need to remove the log2(N) least significant bits from A.
// This means that for 2 byte alignment, we want to remove ONLY the last bits and retain the rest.
// 2 byte alignment -> remove least significant 1 bit.
// 4 byte alignment -> remove least significant 2 bits.
// 8 byte alignment -> remove least significant 3 bits.
// e.g. To align to 2 bytes (0010), we need to do A & (1110). 
// e.g. To align to 4 bytes (0100), we need to do A & (1100). 
// e.g. To align to 8 bytes (1000), we need to do A & (1000).
// and so on and so forth...
// Thus, the 'mask' can be defined as ~(N-1) 
// e.g.  2 bytes -> ~(0010 - 1) = ~0001 = 1110
// e.g.  4 bytes -> ~(0100 - 1) = ~0011 = 1100
// e.g.  8 bytes -> ~(1000 - 1) = ~0111 = 1000
// And thus, the forumla for backward alignment is: A & ~(N-1)
static inline void* 
AlignBackward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)(uptr(ptr) & ~(align - 1));
}

// The functions below return the difference in bytes between the given addresses and 
// the alignment they need to align to (backwards)
// If the forward alignment formula is: (A & ~(N-1))
// Then the formula for the difference is the the original address minus the result: 
// A - (A & ~(N-1))
static inline void* 
AlignForward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)((uptr(ptr) + (align - 1)) & ~(align - 1));
}

static inline u8 
AlignBackwardDiff(void* ptr, u8 align)  {
    return u8((uptr)ptr - uptr(AlignBackward(ptr, align)));
}

static inline u8 
AlignForwardDiff(void* ptr, u8 align)  {
    return u8(uptr(AlignForward(ptr, align)) - uptr(ptr));
}

static inline void 
Copy(void* dest, void* src, u32 size) {
    for (u8 *p = (u8*)dest, *q = (u8*)src, *e = p + size; 
         p < e; ++p, 
         ++q)
    {
        *p = *q;
    }
}

static inline void 
Zero(void *mem, usize size) {
    for (u8 *p = (u8*)mem, *e = p + size; 
         p < e; 
         ++p)
    {
        *p = 0;
    }
}

#define ZeroStruct(p) Zero((p), sizeof(*(p)))
#define ZeroStaticArray(a) Zero((a), sizeof((a)))
#define ZeroDynamicArray(a, c) Zero((a), sizeof(*(a)) * c)


static inline u32
constexpr FourCC(const char String[5]) {
    return 
        ((u32)(String[0]) << 0 ) |
        ((u32)(String[1]) << 8 ) |
        ((u32)(String[2]) << 16) |
        ((u32)(String[3]) << 24);
}

static inline void
EndianSwap(u16* Value) {
    u16 Origin = (*Value);
    (*Value) = ((Origin << 8) | Origin >> 8);
}

static inline void
EndianSwap(u32* Value) {
    u32 Origin = (*Value);
    (*Value) =  ((Origin << 24) |
                 ((Origin & 0xFF00) << 8) |
                 ((Origin >> 8) & 0xFF00) |
                 (Origin >> 24));
}

// TODO(Momo): Delete
static inline void*
Consume(void** P, usize Size) {
    void* Ret = (*P);
    (*P) = (u8*)(*P) + Size; 
    return Ret;
}    

// TODO(Momo): Delete
template<typename t>
static inline t*
Consume(void** P) {
    return (t*)Read(P, sizeof(t));
}

static inline void*
ConsumeBlock(void** P, u32 Size) {
    void* Ret = (*P);
    (*P) = (u8*)(*P) + Size; 
    return Ret;
}
#define ConsumeStruct(Type, Memory) ((Type)*)ConsumeBlock(Memory, sizeof(Type)) 

template<typename T>
static inline void
Write(void** P, T Item) {
    T* LocationAsT = (T*)(*P);
    (*LocationAsT) = Item;
    (*P) = (u8*)(*P) + sizeof(T);
}

#endif 

