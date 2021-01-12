#ifndef __MM_CORE_H__
#define __MM_CORE_H__

// Includes that I sadly cannot live without
#include <cstdint>
#define REFACTOR 1

// Types
using c8 = char;
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
using iptr = ptrdiff_t;

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
#define OffsetOf(Type, Member) (usize)&(((Type*)0)->Member)
#define Ratio(x, min, max) (((x) - (min))/((max) - (min)))
#define Swap(a, b) { auto Temp = (a); (a) = (b); (b) = Temp; }

template<typename T>
union range {
    struct {
        T Start;
        T End;
    };
    struct {
        T Min; 
        T Max;
    };
};


// TODO: rename to 'maybe'
struct no {}; 
template<typename T>
struct maybe {
    T Item;
    b32 IsNone;
  
    maybe(T Item) : Item(Item), IsNone(false) {}
    maybe(no) : IsNone(true) {}

    operator bool() {
        return !IsNone;
    }
};

template<typename T>
static inline maybe<T>
Yes(T Item) {
    return maybe<T>(Item);
}

static inline no
No() {
    return no{};
}


// C-string
static inline u32
SiStrLen(const char* Str) {
    u32 Count = 0;
    for(; (*Str) != 0 ; ++Count, ++Str);
    return Count;
}

static inline void
SiStrCopy(char * Dest, const char* Src) {
    for(; (*Src) != 0 ; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}

static inline void
SiStrConcat(char* Dest, const char* Src) {
    // Go to the end of Dest
    for (; (*Dest) != 0; ++Dest);
    for (; (*Src) != 0; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}


static inline void 
SiStrClear(char* Dest) {
    (*Dest) = 0;
}

static inline void
SiStrReverse(char* Dest) {
    char* BackPtr = Dest;
    for (; *(BackPtr+1) != 0; ++BackPtr);
    for (;Dest < BackPtr; ++Dest, --BackPtr) {
        Swap((*Dest), (*BackPtr));
    }
}


// Memory manipulation
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
SiItoa(char* Dest, i32 Num) {
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

    SiStrReverse(Dest);
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

// Safe Truncation
#define I8_MIN                (-128)
#define I16_MIN               (-32768)
#define I32_MIN               (-2147483648)
#define I64_MIN               (-9223372036854775808)

#define I8_MAX                (127)
#define I16_MAX               (32767)
#define I32_MAX               (2147483647)
#define I64_MAX               (9223372036854775807)

#define U8_MAX                (255)
#define U16_MAX               (65535)
#define U32_MAX               (4294967295)
#define U64_MAX               (18446744073709551615)


static inline u32
SafeCastU64ToU32(u64 Value) {
    Assert(Value <= U32_MAX);
    return (u32)Value;
}

static inline u32
SafeCastI32ToU32(i32 Value) {
    Assert(Value >= 0);
    return (u32)Value;
}

static inline u16
SafeCastI32ToU16(i32 Value) {
    Assert(Value <= U16_MAX && Value >= 0);
    return (u16)Value;
}

static inline u32
SafeCastI64ToU32(i64 Value) {
    Assert(Value <= U32_MAX && Value >= 0);
    return (u32)Value;
}


#endif
