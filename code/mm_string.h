#ifndef __MOMO_STRING__
#define __MOMO_STRING__

#include "mm_core.h"

// 1. No null terminating value stores
// 2. If the operations overflow, it will cap at the capacity.
// TODO: Is it worth writing a struct to contain 'const' strings such that we do not have to deal with const char* anymore in our code base?


struct mms_const_string {
    usize Length;
    const char* Elements;

    inline const auto operator[](usize I) const {
        Assert(I < Length);
        return Elements[I];
    }
};

static inline mms_const_string 
mms_ConstString(const char* Cstr) {
    return mms_const_string {
        CstrLen(Cstr),
        Cstr
    };
}


struct mms_string {
    union {
        mms_const_string String;
        struct {
            usize Length;
            char* Elements;
        };
    };
    usize Capacity;

    inline auto& operator[](usize I) {
        Assert(I < Length);
        return Elements[I];
    }
};



static inline mms_string
mms_String(char* Memory, usize Capacity) {
    Assert(Capacity > 0);
    Assert(Memory);

    mms_string Ret = {};
    Ret.Elements = Memory;
    Ret.Capacity = Capacity;

    return Ret;
}


static inline void
mms_Clear(mms_string* Dest) {
    Dest->Length = 0;
}

static inline void
mms_Copy(mms_string* Dest, mms_const_string Src) {
    Assert(Src.Length <= Dest->Capacity);
    for (u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[i] = Src.Elements[i];
    }
    Dest->Length = Src.Length;
}

static inline void
mms_Copy(mms_string* Dest, mms_string* Src) {
    mms_Copy(Dest, Src->String);
}


static inline void
mms_Push(mms_string* Dest, char Char) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length++] = Char;
}

static inline void
mms_Pop(mms_string* Dest) {
    Assert(Dest->Length > 0);
    --Dest->Length;
}

static inline b32
mms_PopSafely(mms_string* Dest) {
    if (Dest->Length > 0 ){
        --Dest->Length;
        return true;
    }
    return false;
}


static inline void
mms_Concat(mms_string* Dest, mms_const_string Src) {
    Assert(Dest->Length + Src.Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[Dest->Length++] = Src.Elements[i];
    }
}

static inline b32
mms_ConcatSafely(mms_string* Dest, mms_const_string Src) {
    if (Dest->Length + Src.Length <= Dest->Capacity) {
        for ( u32 i = 0; i < Src.Length; ++i ) {
            Dest->Elements[Dest->Length++] = Src.Elements[i];
        }
        return true;
    }
    return false;
}

static inline void
mms_NullTerm(mms_string* Dest) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length] = 0;
}

static inline b32
mms_Compare(mms_const_string Lhs, mms_const_string Rhs) { 
    if(Lhs.Length != Rhs.Length) {
        return false;
    }
    for (u32 i = 0; i < Lhs.Length; ++i) {
        if (Lhs.Elements[i] != Rhs.Elements[i]) {
            return false;
        }
    }
    return true;
}

static inline void
mms_Reverse(mms_string* Dest) {
    for (usize i = 0; i < Dest->Length/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Length-1-i]);
    }
}

static inline void
mms_Itoa(mms_string* Dest, i32 Num) {
    // Naive method. 
    // Extract each number starting from the back and fill the buffer. 
    // Then reverse it.
    
    // Special case for 0
    if (Num == 0) {
        mms_Push(Dest, '0');
        return;
    }

    b32 Negative = Num < 0;
    Num = Abs(Num);

    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        mms_Push(Dest, (char)(DigitToConvert + '0'));
    }

    if (Negative) {
        mms_Push(Dest, '-');
    }

    mms_Reverse(Dest);
}



#include "mm_arena.h"
static inline mms_string
mms_String(mmarn_arena* Arena, usize Capacity) {
    char* Buffer = mmarn_PushArray<char>(Arena, Capacity); 
    return mms_String(Buffer, Capacity);
}

static inline mms_string* 
mms_PushString(mmarn_arena* Arena, usize Capacity) {
    mms_string* Ret = mmarn_PushStruct<mms_string>(Arena);
    (*Ret) = mms_String(Arena, Capacity);
    return Ret;
}

#endif
