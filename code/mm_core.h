#ifndef __RYOJI_H__
#define __RYOJI_H__

// Includes
#include <cstdint>

// Types
using b8 = bool;
using b32 = uint32_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using usize = size_t;
using uptr = uintptr_t;


#define Kilobyte (1 << 10)
#define Megabyte (1 << 20)
#define Gigabyte (1 << 30)

#define Abs(x) (((x) < 0) ? -(x) : (x))
#define Maximum(x, y) (((x) > (y)) ? (x) : (y))
#define Minimum(x, y) (((x) < (y)) ? (x) : (y))
#define Kilobytes(num) (Kilobyte * num)
#define Megabytes(num) (Megabyte * num)
#define Gigabytes(num) (Gigabyte * num)
#define ArrayCount(arr) (sizeof(arr)/sizeof(*arr))
#define Clamp(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define TwoToOne(row, col, width) (col + row * width) 
#define Complement(x, low, high) (high - x + low)
#define Mask(flag, mask) (flag | mask)
#define Unmask(flag, mask) (flag & ~mask)
#define IsMasked(flag, mask) ((flag & mask) > 0)
#define Lerp(start, end, fraction) ((start) + ((end) - (start))*(fraction)) 
#define OffsetOf(type, Member) (usize)&(((type *)0)->Member)
#define Ratio(x, min, max) (((x) - (min))/((max) - (min)))
#define Swap(a, b) { auto Temp = (a); (a) = (b); (b) = Temp; }

static inline u32
StrLen(const char* Str) {
    u32 Count = 0;
    for(; (*Str) != 0 ; ++Count, ++Str);
    return Count;
}

static inline void
CstrCopy(char * Dest, const char* Src) {
    for(; (*Src) != 0 ; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}

static inline void
CstrConcat(char* Dest, const char* Src) {
    // Go to the end of Dest
    for (; (*Dest) != 0; ++Dest);
    for (; (*Src) != 0; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}


static inline void 
StrClear(char* Dest) {
    (*Dest) = 0;
}

static inline void
StrReverse(char* Dest) {
    char* BackPtr = Dest;
    for (; *(BackPtr+1) != 0; ++BackPtr);
    for (;Dest < BackPtr; ++Dest, --BackPtr) {
        Swap((*Dest), (*BackPtr));
    }
}



static inline void 
MemCopy(void* dest, void* src, usize size) {
    for (u8 *p = (u8*)dest, *q = (u8*)src, *e = p + size; p < e; ++p, ++q){
        *p = *q;
    }
}

static inline void 
MemZero(void *mem, usize size) {
    for (u8 *p = (u8*)mem, *e = p + size; p < e; ++p){
        *p = 0;
    }
}

static inline void 
Itoa(char* Dest, i32 Num) {
    // Naive method. 
    // Extract each number starting from the back and fill the buffer. 
    // Then reverse it.
    
    // Special case for 0
    if (Num == 0) {
        Dest[0] = '0';
        Dest[1] = 0;
        return;
    }

    b32 Negative = Num < 0;
    Num = Abs(Num);

    char* It = Dest;
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        *(It++) = (char)(DigitToConvert + '0');
    }

    if (Negative) {
        *(It++) = '-';
    }
    (*It) = 0;

    StrReverse(Dest);
}

#define ZeroMem(m, s) MemZero(m, s)
#define ZeroStruct(p) ZeroMem((p), sizeof(*(p)))
#define ZeroStaticArray(a) ZeroMem((a), sizeof((a)))
#define ZeroDynamicArray(a, c) ZeroMem((a), sizeof(*(a)) * c)

// Assertion
#if SLOW
#include <assert.h>
#define Assert(x) assert(x)
//#define Assert(x) {if(!(x)) {*(volatile int *)0 = 0;}}
#else
#define Assert(x) 
#endif


// Run-time system endianness check
static inline b32 
IsSystemBigEndian() {
    int n = 1;
    return (*(char*)&n != 1);
}

// Defer 
namespace zawarudo {
    template<class F> struct ScopeGuard {
        F f;
        ~ScopeGuard() { f(); }
    };
    struct defer_dummy {};
    template<class F> ScopeGuard<F> operator+(defer_dummy, F f) {
        return { f };
    }
}

#define zawarudo_VARANON_IMPL(COUNTER) zawarudo_ryojianon##COUNTER
#define zawarudo_VARANON(counter) zawarudo_VARANON_IMPL(counter)
#define Defer auto zawarudo_VARANON(__COUNTER__) = zawarudo::defer_dummy{} + [&]()

#endif
