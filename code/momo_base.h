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
typedef uintptr_t umi;
typedef ptrdiff_t smi;


#define KIBIBYTE (1 << 10)
#define MEBIBYTE (1 << 20)
#define GIBIBYTE (1 << 30)
#define KIBIBYTES(num) (KIBIBYTE * num)
#define MEBIBYTES(num) (MEBIBYTE * num)
#define GIBIBYTES(num) (GIBIBYTE * num)
#define ArrayCount(arr) (sizeof(arr)/sizeof(*arr))

#define _Glue(A,B) A##B
#define Glue(A,B) _Glue(A,B)

#define Abs(X) (((X) < 0) ? (-(X)) : (X))
#define Max(X,Y) ((X) > (Y) ? (X) : (Y))
#define Min(X,Y) ((X) < (Y) ? (X) : (Y))


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
#define Swap(A,B) do{ auto Glue(zawarudo, __LINE__) = (A); (A) = (B); (B) = Glue(zawarudo, __LINE__); } while(0);
#define Clamp(Value, Low, High) Max(Min(Value, High), Low)
#define OffsetOf(Type, Member) (umi)&(((Type*)0)->Member)
#define Lerp(Start,End,Fraction) (Start) + (((End) - (Start)) * (Fraction))



// C-string
static inline u32
Sistr_Length(const char* Str) {
    u32 Count = 0;
    for(; (*Str) != 0 ; ++Count, ++Str);
    return Count;
}

static inline void
Sistr_Copy(char * Dest, const char* Src) {
    for(; (*Src) != 0 ; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}

static inline b8
Sistr_Compare(const char* Lhs, const char* Rhs) {
    for(; (*Rhs) != 0 ; ++Rhs, ++Lhs) {
        if ((*Lhs) != (*Rhs)) {
            return false;
        }
    }
    return true;
}

static inline b8
Sistr_CompareN(const char* Lhs, const char* Rhs, umi N) {
    for(umi I = 0; I < N; ++I, ++Lhs, ++Rhs) {
        if ((*Lhs) != (*Rhs)) {
            return false;
        }
    }
    return true;
}
static inline void
Sistr_Concat(char* Dest, const char* Src) {
    // Go to the end of Dest
    for (; (*Dest) != 0; ++Dest);
    for (; (*Src) != 0; ++Src, ++Dest) {
        (*Dest) = (*Src);
    }
    (*Dest) = 0;
}


static inline void 
Sistr_Clear(char* Dest) {
    (*Dest) = 0;
}

static inline void
Sistr_Reverse(char* Dest) {
    char* BackPtr = Dest;
    for (; *(BackPtr+1) != 0; ++BackPtr);
    for (;Dest < BackPtr; ++Dest, --BackPtr) {
        Swap(*Dest, *BackPtr);
    }
}



static inline void 
Sistr_Itoa(char* Dest, s32 Num) {
    // Naive method. 
    // Extract each number starting from the back and fill the buffer. 
    // Then reverse it.
    
    // Special case for 0
    if (Num == 0) {
        Dest[0] = '0';
        Dest[1] = 0;
        return;
    }
    
    b8 Negative = Num < 0;
    Num = Abs(Num);
    
    char* It = Dest;
    for(; Num != 0; Num /= 10) {
        s32 DigitToConvert = Num % 10;
        *(It++) = (char)(DigitToConvert + '0');
    }
    
    if (Negative) {
        *(It++) = '-';
    }
    (*It) = 0;
    
    Sistr_Reverse(Dest);
}


// ASSERTion
#if SLOW
#include <assert.h>
#define ASSERT(x) assert(x)
//#define ASSERT(x) {if(!(x)) {*(volatile int *)0 = 0;}}
#else
#define ASSERT(x) 
#endif


// defer 
template<class F> struct zawarudo_ScopeGuard {
    F f;
    ~zawarudo_ScopeGuard() { f(); }
};
struct zawarudo_defer_dummy {};
template<class F> zawarudo_ScopeGuard<F> operator+(zawarudo_defer_dummy, F f) {
    return { f };
}

#define zawarudo_AnonVarSub(x) zawarudo_defer##x
#define zawarudo_AnonVar(x) zawarudo_AnonVarSub(x)
#define defer auto zawarudo_AnonVar(__LINE__) = zawarudo_defer_dummy{} + [&]()

// Safe Truncation
#define I8_MIN                (-128)
#define I16_MIN              (-32768)
#define I32_MIN               (-2147483648)
#define I64_MIN               (-9223372036854775808)

#define I8_MAX                (127)
#define I16_MAX               (32767)
#define I32_MAX               (2147483647)
#define I64_MAX               (9223372036854775807)

#define U8_MAX               (255)
#define U16_MAX               (65535)
#define U32_MAX               (4294967295)
#define U64_MAX               (18446744073709551615)


//~ NOTE(Momo): Converts single digits 
static inline u8
DigitToAscii(u8 digit){
    ASSERT(digit >= 0 && digit <= 9);
    return digit + '0';
}
static inline u8
AsciiToDigit(u8 ascii){
    ASSERT(ascii >= '0' && ascii <= '9');
    return ascii - '0';
}

#endif //MM_BASIC_TYPES_H
