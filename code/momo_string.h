#ifndef MM_STRING
#define MM_STRING


//~ NOTE(Momo): u8_cstr
// TODO(Momo): Support for UTF8??
struct u8_cstr {
    u8* Data;
    u32 Count;
};

static inline b8
U8CStr_Init(u8_cstr* S, u8* Data, u32 Count) {
    if (!Data || Count == 0) {
        return false;
    }
    S->Data = Data;
    S->Count = Count;
    
    return true;
}

static inline u8_cstr
U8CStr_Create(u8* Data, u32 Count) {
    u8_cstr S = {};
    S.Data = Data;
    S.Count = Count;
    
    return S;
}

// Assumes C-String
static inline u8_cstr
U8CStr_CreateFromSiStr(const char* SiStr) {
    return U8CStr_Create((u8*)SiStr, SiStrLen(SiStr));
    
}

static inline b8
U8CStr_InitFromSiStr(u8_cstr* S, const char* SiStr) {
    return U8CStr_Init(S, (u8*)SiStr, SiStrLen(SiStr));
}

static inline b8
U8CStr_Cmp(u8_cstr Lhs, u8_cstr Rhs) {
    if(Lhs.Count != Rhs.Count) {
        return false;
    }
    for (u32 I = 0; I < Lhs.Count; ++I) {
        if (Lhs.Data[I] != Rhs.Data[I]) {
            return false;
        }
    }
    return true;
}


static inline b8
U8CStr_CmpSiStr(u8_cstr Lhs, const char* Rhs) {
    for(u32 I = 0; I < Lhs.Count; ++I) {
        if (Lhs.Data[I] != Rhs[I]) {
            return false;
        }
    }
    return true;
}

static inline void
U8CStr_SubString(u8_cstr* Dest, u8_cstr Src, u32 Min, u32 Max) {
    Assert(Min <= Max); 
    U8CStr_Init(Dest, Src.Data + Min, Max - Min);
}

struct u8_cstr_split_res {
    u8_cstr* Items;
    u32 ItemCount;
};

static inline u32
U8CStr_Find(u8_cstr Str, u8 Item, u32 StartIndex) {
    for(u32 I = StartIndex; I < Str.Count; ++I) {
        if(Str.Data[I] == Item) {
            return I;
        }
    }
    return Str.Count;
}

static inline u8_cstr_split_res
U8CStr_SplitByDelimiter(u8_cstr Str, arena* Arena, u8 Delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<string> struct.
    u8_cstr_split_res Ret = {};
    u32 Min = 0;
    u32 Max = 0;
    
    for (;Max != Str.Count;) {
        Max = U8CStr_Find(Str, Delimiter, Min);
        
        u8_cstr* Link = Arena_PushStruct(u8_cstr, Arena);
        Assert(Link);
        U8CStr_SubString(Link, Str, Min, Max);
        
        if (Ret.Items == nullptr) {
            Ret.Items = Link;            
        }
        
        Min = Max + 1;
        ++Ret.ItemCount;
    }
    return Ret;
}

//~ NOTE(Momo): u8_str
struct u8_str {
    union {
        u8_cstr CStr;
        struct {
            u8* Data;
            u32 Count;
        };
    };
    u32 Cap;
};

static inline void
U8Str_Init(u8_str* S, u8* Buffer, u32 Capacity) {
    S->Data = Buffer;
    S->Count = 0;
    S->Cap = Capacity;
}


static inline b8
U8Str_New(u8_str* S, arena* Arena, u32 Capacity) {
    u8* Buffer = Arena_PushArray(u8, Arena, Capacity);
    if(!Buffer) {
        return false;
    }
    S->Data = Buffer;
    S->Count = 0;
    S->Cap = Capacity;
    
    return true;
}

static inline b8
U8Str_Pop(u8_str* S) {
    if (S->Count <= 0) {
        return false;
    }
    --S->Count;
    return true;
}

static inline u32 
U8Str_Remaining(u8_str* Buffer) {
    return Buffer->Cap - Buffer->Count;
}

static inline b8
U8Str_CopyCStr(u8_str* Dest, u8_cstr Src) {
    if (Src.Count > Dest->Cap) {
        return false;
    }
    for (u32 I = 0; I < Src.Count; ++I ) {
        Dest->Data[I] = Src.Data[I];
    }
    Dest->Count = Src.Count;
    return true;
}

static inline b8
U8Str_Copy(u8_str* Dest, u8_str* Src) {
    return U8Str_CopyCStr(Dest, Src->CStr);
}


static inline b8
U8Str_NullTerm(u8_str* Dest) {
    if (Dest->Count < Dest->Cap) {
        Dest->Data[Dest->Count] = 0;
        return true;
    }
    return false;
}

static inline b8
U8Str_Push(u8_str* Dest, u8 Item) {
    if (Dest->Count < Dest->Cap) {
        Dest->Data[Dest->Count++] = Item;
        return true;
    }
    return false;
}

static inline b8
U8Str_PushCStr(u8_str* Dest, u8_cstr Src) {
    if (Dest->Count + Src.Count <= Dest->Cap) {
        for ( u32 I = 0; I < Src.Count; ++I ) {
            Dest->Data[Dest->Count++] = Src.Data[I];
        }
        return true;
    }
    return false;
}

static inline b8
U8Str_PushStr(u8_str* Dest, u8_str* Src) {
    return U8Str_PushCStr(Dest, Src->CStr);
}

static inline void 
U8Str_Clear(u8_str* Dest) {
    Dest->Count = 0;
}

static inline b8
U8Str_PushU32(u8_str* Dest, u32 Num) {
    if (Num == 0) {
        U8Str_Push(Dest, '0');
        return true;
    }
    u32 StartPoint = Dest->Count; 
    
    for(; Num != 0; Num /= 10) {
        s32 DigitToConvert = Num % 10;
        b8 Success = U8Str_Push(Dest, (u8)(DigitToConvert + '0'));
        if (!Success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, 
             Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Count - 1 - I]);
    }
    return true;
}


static inline b8
U8Str_PushS32(u8_str* Dest, s32 Num) {
    if (Num == 0) {
        if(!U8Str_Push(Dest, '0')) {
            return false;
        }
        return true;
    }
    
    u32 StartPoint = Dest->Count; 
    
    b8 Negative = Num < 0;
    Num = AbsOf(Num);
    
    for(; Num != 0; Num /= 10) {
        s32 DigitToConvert = Num % 10;
        U8Str_Push(Dest, (char)(DigitToConvert + '0'));
    }
    
    if (Negative) {
        U8Str_Push(Dest, '-');
    }
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Count-1-I]);
        
    }
}


#endif
