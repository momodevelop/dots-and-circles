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
StrCopy(char * Dest, const char* Str) {
    for(; (*Str) != 0 ; ++Str, ++Dest) {
        (*Dest) = (*Str);
    }
    (*Dest) = 0;
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

// Enum string generator
#define zawarudo_GENERATE_ENUM_PART(ENUM) ENUM,
#define zawarudo_GENERATE_SWITCH_CASE_PART(STR) case STR: return #STR;
#define GenerateEnumStrings(ENUM_NAME, FUNC_NAME, FOREACH) \
enum ENUM_NAME { \
FOREACH(zawarudo_GENERATE_ENUM_PART) \
};\
static inline const char* FUNC_NAME(ENUM_NAME e) { \
switch(e) { \
FOREACH(zawarudo_GENERATE_SWITCH_CASE_PART) \
default: return "Unknown Error :("; \
} \
}\



#endif
