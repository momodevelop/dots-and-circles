#ifndef MM_BASIC_TYPES_H
#define MM_BASIC_TYPES_H

// Types
typedef char c8;
typedef bool b8;
typedef uint32_t b32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef uintptr_t uptr;
typedef ptrdiff_t iptr;

// TODO(Momo): change to retarded name
#define Kibibyte (1 << 10)
#define Mebibyte (1 << 20)
#define Gibibyte (1 << 30)
#define Kibibytes(num) (Kibibyte * num)
#define Megibytes(num) (Mebibyte * num)
#define Gibibytes(num) (Gibibyte * num)
#define ArrayCount(arr) (sizeof(arr)/sizeof(*arr))

#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)

#define AbsOf(X) (((X) < 0) ? (-(X)) : (X))
#define MaxOf(X,Y) ((X) > (Y) ? (X) : (Y))
#define MinOf(X,Y) ((X) < (Y) ? (X) : (Y))

// NOTE(Momo): We need to do/while to cater for if/else statements
// that looks like this:
// >> if (...) 
// >>     Swap(...); 
// >> else 
// >>     ...
// because it will expand to:
// >> if (...) 
// >>    {...}; 
// >> else 
// >>    ...
// which causes an invalid ';' error
#define Swap(T,A,B) do{ T Glue(zawarudo, __LINE__) = (A); (A) = (B); (B) = Glue(zawarudo, __LINE__); } while(0);
#define Clamp(Value, Low, High) MaxOf(MinOf(Value, High), Low)
#define OffsetOf(Type, Member) (usize)&(((Type*)0)->Member)
#define Lerp(Start,End,Fraction) (Start) + (((End) - (Start)) * (Fraction))

// Get the ratio of Value within the range [Min,Max] 
// Return value Will be [0, 1]
static inline f32
Ratio(f32 Value, f32 Min, f32 Max) {
    return (Value - Min)/(Max - Min); 
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
        Swap(char, *Dest, *BackPtr);
    }
}



static inline void 
SiStrItoa(char* Dest, s32 Num) {
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
    Num = AbsOf(Num);
    
    char* It = Dest;
    for(; Num != 0; Num /= 10) {
        s32 DigitToConvert = Num % 10;
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
U64_ToU32(u64 Value) {
    Assert(Value <= U32_Max);
    return (u32)Value;
}

static inline u32
S32_ToU32(s32 Value) {
    Assert(Value >= 0);
    return (u32)Value;
}

static inline u16
S32_ToU16(s32 Value) {
    Assert(Value <= U16_Max && Value >= 0);
    return (u16)Value;
}

static inline u32
S64_ToU32(s64 Value) {
    Assert(Value <= U32_Max && Value >= 0);
    return (u32)Value;
}

#endif //MM_BASIC_TYPES_H
