#ifndef __MM_CORE_H__
#define __MM_CORE_H__

// Includes that I sadly cannot live without
#include <cstdint>
#define REFACTOR 0



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
#define Kilobytes(num) (Kilobyte * num)
#define Megabytes(num) (Megabyte * num)
#define Gigabytes(num) (Gigabyte * num)
#define ArrayCount(arr) (sizeof(arr)/sizeof(*arr))

template<typename t>
static inline t
Abs(t X) {
    return X < 0 ? -X : X;
}

template<typename t>
static inline t
Max(t X, t Y) {
    return X > Y ? X : Y;
}

template<typename t>
static inline t
Min(t X, t Y) {
    return X < Y ? X : Y;
}

template<typename t>
static inline void
Swap(t* A, t* B) {
    t Temp = (*A);
    (*A) = (*B);
    (*B) = Temp;
}

template<typename t>
static inline t
Clamp(t Value, t Low, t High) {
    return Min(Max(Value, High), Low); 
}

template<typename t>
static inline t
Lerp(t Start, t End, f32 Fraction) {
    return Start + ((End - Start) * Fraction);
}

// Get the ratio of Value within the range [Min,Max] 
// Return value Will be [0, 1]
static inline f32
Ratio(f32 Value, f32 Min, f32 Max) {
    return (Value - Min)/(Max - Min); 
}

// TODO: Change the rest to templates
#define OffsetOf(Type, Member) (usize)&(((Type*)0)->Member)

template<typename t>
union range {
    struct {
        t Start;
        t End;
    };
    struct {
        t Min; 
        t Max;
    };
};


struct no {}; 
template<typename t>
struct maybe {
    t This;
    b32 IsNone;
  
    maybe(t This) : This(This), IsNone(false) {}
    maybe(no) : IsNone(true) {}

    operator bool() {
        return !IsNone;
    }
};

template<typename t>
static inline maybe<t>
Yes(t Item) {
    return maybe<t>(Item);
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

static inline b8
SiStrCompare(const char* Lhs, const char* Rhs) {
    for(; (*Rhs) != 0 ; ++Rhs, ++Lhs) {
        if ((*Lhs) != (*Rhs)) {
            return false;
        }
    }
    return true;
}

static inline b8
SiStrCompareN(const char* Lhs, const char* Rhs, usize N) {
    for(usize I = 0; I < N; ++I, ++Lhs, ++Rhs) {
        if ((*Lhs) != (*Rhs)) {
            return false;
        }
    }
    return true;
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
        Swap(Dest, BackPtr);
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

#define AnonVarSub(x) zawarudo_ryojianon##x
#define AnonVar(x) AnonVarSub(x)
#define Defer auto AnonVar(__COUNTER__) = zawarudo::defer_dummy{} + [&]()

// Safe Truncation
#define I8_Min                (-128)
#define I16_Min              (-32768)
#define I32_Min               (-2147483648)
#define I64_Min               (-9223372036854775808)

#define I8_Max                (127)
#define I16_Max               (32767)
#define I32_Max               (2147483647)
#define I64_Max               (9223372036854775807)

#define U8_Max               (255)
#define U16_Max               (65535)
#define U32_Max               (4294967295)
#define U64_Max               (18446744073709551615)


static inline u32
SafeCastU32(u64 Value) {
    Assert(Value <= U32_Max);
    return (u32)Value;
}

static inline u32
SafeCastU32(i32 Value) {
    Assert(Value >= 0);
    return (u32)Value;
}

static inline u16
SafeCastU16(i32 Value) {
    Assert(Value <= U16_Max && Value >= 0);
    return (u16)Value;
}

static inline u32
SafeCastU32(i64 Value) {
    Assert(Value <= U32_Max && Value >= 0);
    return (u32)Value;
}



#endif
