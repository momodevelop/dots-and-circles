#ifndef __RYOJI_H__
#define __RYOJI_H__

// Includes
#include <cstdint>

// Types
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

#define KILOBYTE (1 << 10)
#define MEGABYTE (1 << 20)
#define GIGABYTE (1 << 30)

#define Abs(x) ((x < 0) ? -x : x)
#define Max(x, y) ((x > y) x : y)
#define Min(x, y) ((x < y) x : y)
#define Kilobytes(num) (KILOBYTE * num)
#define Megabytes(num) (MEGABYTE * num)
#define Gigabytes(num) (GIGABYTE * num)
#define ArrayCount(arr) (sizeof(arr)/sizeof(*arr))
#define Clamp(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define TwoToOne(row, col, width) (col + row * width) 
#define Complement(x, low, high) (high - x + low)
#define Wrap(x, low, high) while(x > high)x-=(high)-(low); while(x < low)x+=(high)-(low);
#define Mask(flag, mask) (flag | mask)
#define Unmask(flag, mask) (flag & ~mask)
#define IsMasked(flag, mask) ((flag & mask) > 0)

static inline void 
ZeroBlock(void *mem, usize size) {
    for (u8 *p = (u8*)mem, *e = p + size; p < e; ++p){
        *p = 0;
    }
}
#define ZeroStruct(p) ZeroBlock((p), sizeof(*(p)))
#define ZeroStaticArray(a) ZeroBlock((a), sizeof((a)))
#define ZeroDynamicArray(a, c) ZeroBlock((a), sizeof(*(a)) * c)



// Assertion
#if SLOW_MODE
#include <assert.h>
#define Assert(x) assert(x)
//#define Assert(x) {if(!(x)) {*(volatile int *)0 = 0;}}
#else
#define Assert(x) 
#endif


// Defer 
template<class F> struct ScopeGuard {
    F f;
    ~ScopeGuard() { f(); }
};

namespace zawarudo {
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
