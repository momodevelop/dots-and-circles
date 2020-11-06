#ifndef __MOMO_STRING__
#define __MOMO_STRING__

#include "mm_core.h"

// 1. No null terminating value stores
// 2. If the operations overflow, it will cap at the capacity.
// TODO: Is it worth writing a struct to contain 'const' strings such that we do not have to deal with const char* anymore in our code base?


struct mms_string {
    usize Length;
    char* E;
};

static inline mms_string 
mms_String(char* Cstr) {
    return mms_string {
        StrLen(Cstr),
        Cstr
    };
}


static inline mms_string
mms_String(const char* Literal) {
    return mms_string {
        StrLen(Literal),
        (char*)Literal
    };
}

struct mms_string_buffer {
    union {
        mms_string String;
        struct {
            usize Length;
            char* E;
        };
    };
    usize Capacity;
};



static inline mms_string_buffer
mms_StringBuffer(char* Memory, usize Capacity) {
    Assert(Capacity > 0);
    Assert(Memory);

    mms_string_buffer Ret = {};
    Ret.E = Memory;
    Ret.Capacity = Capacity;

    return Ret;
}


static inline void
mms_Clear(mms_string_buffer* Dest) {
    Dest->Length = 0;
}

static inline void
mms_Copy(mms_string_buffer* Dest, mms_string Src) {
    Assert(Dest->Length <= Src.Length);
    for (u32 i = 0; i < Src.Length; ++i ) {
        Dest->E[i] = Src.E[i];
    }
    Dest->Length = Src.Length;
}

static inline void
mms_Push(mms_string_buffer* Dest, char Char) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->E[Dest->Length++] = Char;
}

static inline char
mms_Pop(mms_string_buffer* Dest) {
    Assert(Dest->Length > 0);
    return Dest->E[--Dest->Length];
}

static inline void
mms_Concat(mms_string_buffer* Dest, mms_string Src) {
    Assert(Dest->Length + Src.Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Length; ++i ) {
        Dest->E[Dest->Length++] = Src.E[i];
    }
}

static inline void
mms_NullTerm(mms_string_buffer* Dest) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->E[Dest->Length] = 0;
}

static inline b32
mms_Compare(mms_string Lhs, mms_string Rhs) { 
    if(Lhs.Length != Rhs.Length) {
        return false;
    }
    for (u32 i = 0; i < Lhs.Length; ++i) {
        if (Lhs.E[i] != Rhs.E[i]) {
            return false;
        }
    }
    return true;
}

#include "mm_arena.h"
static inline mms_string_buffer
mms_PushString(mmarn_arena* Arena, usize Capacity) {
    char* Buffer = mmarn_PushArray<char>(Arena, Capacity); 
    mms_string_buffer Ret = mms_StringBuffer(Buffer, Capacity);
    return Ret;
}

#endif
