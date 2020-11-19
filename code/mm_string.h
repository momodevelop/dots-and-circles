#ifndef __MOMO_STRING__
#define __MOMO_STRING__

#include "mm_core.h"

// 1. No null terminating value stores
// 2. If the operations overflow, it will cap at the capacity.
// TODO: Is it worth writing a struct to contain 'const' strings such that we do not have to deal with const char* anymore in our code base?



struct mms_string {
    struct {
        usize Length;
        char* Elements;
    };
    inline auto& operator[](usize I) const {
        Assert(I < Length);
        return Elements[I];
    }
};


static inline mms_string 
mms_String(char* Elements, usize Length) {
    Assert(Elements != nullptr);
    mms_string Ret = {};
    Ret.Elements = Elements;
    Ret.Length = Length;
    return Ret;
}


// Assumes C-String
static inline mms_string 
mms_String(char* Cstr) {
    return mms_String(Cstr, CstrLen(Cstr));
}

// Assumes C-String
static inline mms_string 
mms_String(const char* Cstr) {
    return mms_String((char*)Cstr);
}

struct mms_string_buffer {
    mms_string String;
    usize Capacity;
    struct {
        usize Length;
        char* Elements;
    };

    inline auto& operator[](usize I) {
        Assert(I < Length);
        return Elements[I];
    }
};



static inline mms_string_buffer
mms_StringBuffer(char* Memory, usize Capacity) {
    Assert(Capacity > 0);
    Assert(Memory);

    mms_string_buffer Ret = {};
    Ret.Elements = Memory;
    Ret.Capacity = Capacity;

    return Ret;
}


static inline void
mms_Clear(mms_string_buffer* Dest) {
    Dest->Length = 0;
}

static inline void
mms_Copy(mms_string_buffer* Dest, mms_string Src) {
    Assert(Src.Length <= Dest->Capacity);
    for (u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[i] = Src.Elements[i];
    }
    Dest->Length = Src.Length;
}



static inline void
mms_Push(mms_string_buffer* Dest, char Char) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length++] = Char;
}

static inline void
mms_Pop(mms_string_buffer* Dest) {
    Assert(Dest->Length > 0);
    --Dest->Length;
}

static inline b32
mms_PopSafely(mms_string_buffer* Dest) {
    if (Dest->Length > 0 ){
        --Dest->Length;
        return true;
    }
    return false;
}


static inline void
mms_Concat(mms_string_buffer* Dest, mms_string Src) {
    Assert(Dest->Length + Src.Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[Dest->Length++] = Src.Elements[i];
    }
}

static inline b32
mms_ConcatSafely(mms_string_buffer* Dest, mms_string Src) {
    if (Dest->Length + Src.Length <= Dest->Capacity) {
        for ( u32 i = 0; i < Src.Length; ++i ) {
            Dest->Elements[Dest->Length++] = Src.Elements[i];
        }
        return true;
    }
    return false;
}

static inline void
mms_NullTerm(mms_string_buffer* Dest) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length] = 0;
}

static inline b32
mms_Compare(mms_string Lhs, mms_string Rhs) { 
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
mms_Reverse(mms_string_buffer* Dest) {
    for (usize i = 0; i < Dest->Length/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Length-1-i]);
    }
}

static inline void
mms_Itoa(mms_string_buffer* Dest, i32 Num) {
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
struct range {
    usize Start;
    usize OnePastEnd;
};


static inline range
mms_FindNextToken(mms_string String, char Delimiter, range Slice = {}) {
  
    if (Slice.OnePastEnd != 0) {
        Slice.Start = Slice.OnePastEnd + 1;
        if (Slice.Start >= String.Length) {
            return Slice;
        }

    }
    for (usize i = Slice.Start; ; ++i) {
         if (i == String.Length || String[i] == Delimiter) {
            Slice.OnePastEnd = i;
            return Slice;
         }
    }

    return Slice;


}

#include "mm_arena.h"
static inline mms_string_buffer
mms_StringBuffer(mmarn_arena* Arena, usize Capacity) {
    char* Buffer = mmarn_PushArray<char>(Arena, Capacity); 
    return mms_StringBuffer(Buffer, Capacity);
}

static inline mms_string_buffer* 
mms_PushStringBuffer(mmarn_arena* Arena, usize Capacity) {
    mms_string_buffer* Ret = mmarn_PushStruct<mms_string_buffer>(Arena);
    (*Ret) = mms_StringBuffer(Arena, Capacity);
    return Ret;
}



#endif
