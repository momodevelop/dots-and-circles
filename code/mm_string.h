#ifndef __MOMO_STRING__
#define __MOMO_STRING__

#include "mm_core.h"

// 1. No null terminating value stores
// 2. If the operations overflow, it will cap at the capacity.


struct mms_string {
    u32 Length;
    u32 Capacity;
    char* Buffer;
    inline const auto& operator[](usize index) const { return Buffer[index]; }
    inline auto& operator[](usize index) { return Buffer[index];}
};


static inline mms_string
mms_CreateString(char* Buffer, u32 Capacity) {
    Assert(Capacity > 0);
    mms_string String = {};
    String.Buffer = Buffer;
    String.Capacity = Capacity;

    return String;
}

static inline void
mms_Clear(mms_string* Dest) {
    Dest->Length = 0;
    Dest->Buffer[0] = 0;
}

static inline void
mms_Copy(mms_string* Dest, const mms_string* Src) {
    Assert(Dest->Capacity <= Src->Capacity);
    for (u32 i = 0; i < Src->Length; ++i ) {
        Dest->Buffer[i] = Src->Buffer[i];
    }
    Dest->Length = Src->Length;
}

static inline void
mms_Copy(mms_string* Dest, const char* Src) {
    Dest->Length = 0;
    for(; (*Src) != 0 ; ++Src) {
        Assert(Dest->Length < Dest->Capacity); 
        Dest->Buffer[Dest->Length++] = (*Src);
    }
}

static inline void
mms_Push(mms_string* Dest, char Char) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Buffer[Dest->Length++] = Char;
}

static inline char
mms_Pop(mms_string* Dest) {
    Assert(Dest->Length > 0);
    return Dest->Buffer[--Dest->Length];
}

static inline void 
mms_Concat(mms_string* Dest, const char* Src) {
    for(; (*Src) != 0 ; ++Src) {
        Assert(Dest->Length < Dest->Capacity); 
        Dest->Buffer[Dest->Length++] = (*Src);
    }
}


static inline void 
mms_Concat(mms_string* Dest, const mms_string* Src) {
    Assert(Dest->Length + Src->Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src->Length; ++i ) {
        Dest->Buffer[Dest->Length++] = Src->Buffer[i];
    }
}

static inline void
mms_NullTerm(mms_string* Dest) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Buffer[Dest->Length] = 0;
}


#endif
