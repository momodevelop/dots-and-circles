#ifndef MM_STRING
#define MM_STRING


//~ NOTE(Momo): u8_cstr
// TODO(Momo): Support for UTF8??
struct u8_cstr {
    u8* Data;
    u32 Count;
};

static inline u8_cstr
U8CStr_Create(u8* Data, u32 Count) {
    u8_cstr Ret = {};
    Ret.Data = Data;
    Ret.Count = Count;
    
    return Ret;
}

// Assumes C-String
static inline u8_cstr
U8CStr_FromSiStr(const char* SiStr) {
    return U8CStr_Create((u8*)SiStr, SiStrLen(SiStr));
}

static inline b32
U8CStr_Compare(u8_cstr Lhs, u8_cstr Rhs) {
    if(Lhs.Count != Rhs.Count) {
        return False;
    }
    for (u32 I = 0; I < Lhs.Count; ++I) {
        if (Lhs.Data[I] != Rhs.Data[I]) {
            return False;
        }
    }
    return True;
}


static inline u8_cstr
U8CStr_SubString(u8_cstr Src, u32 Min, u32 Max) {
    Assert(Min <= Max); 
    return U8CStr_Create(Src.Data + Min, Max - Min);
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
        (*Link) = U8CStr_SubString(Str, Min, Max);
        
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

static inline u8_str
U8Str_Create(u8* Buffer, u32 Capacity) {
    u8_str Ret = {};
    Ret.Data = Buffer;
    Ret.Count = 0;
    Ret.Cap = Capacity;
    return Ret;
}
#define U8Str_CreateFromBuffer(Buffer) U8Str_Create(Buffer, sizeof(Buffer))

static inline u8_str
U8Str_CreateFromArena(arena* Arena, u32 Capacity) {
    u8* Buffer = Arena_PushArray(u8, Arena, Capacity);
    return U8Str_Create(Buffer, Capacity);
}

static inline b32
U8Str_Pop(u8_str* S) {
    if (S->Count <= 0) {
        return False;
    }
    --S->Count;
    return True;
}

static inline u32 
U8Str_Remaining(u8_str* Buffer) {
    return Buffer->Cap - Buffer->Count;
}

static inline b32
U8Str_Copy(u8_str* Dest, u8_cstr Src) {
    if (Src.Count > Dest->Cap) {
        return False;
    }
    for (u32 I = 0; I < Src.Count; ++I ) {
        Dest->Data[I] = Src.Data[I];
    }
    Dest->Count = Src.Count;
    return True;
}

static inline b32
U8Str_NullTerm(u8_str* Dest) {
    if (Dest->Count < Dest->Cap) {
        Dest->Data[Dest->Count] = 0;
        return True;
    }
    return False;
}

static inline b32
U8Str_Push(u8_str* Dest, u8 Item) {
    if (Dest->Count < Dest->Cap) {
        Dest->Data[Dest->Count++] = Item;
        return True;
    }
    return False;
}

static inline b32
U8Str_PushCStr(u8_str* Dest, u8_cstr Src) {
    if (Dest->Count + Src.Count <= Dest->Cap) {
        for ( u32 I = 0; I < Src.Count; ++I ) {
            Dest->Data[Dest->Count++] = Src.Data[I];
        }
        return True;
    }
    return False;
}

static inline b32
U8Str_PushStr(u8_str* Dest, u8_str* Src) {
    return U8Str_PushCStr(Dest, Src->CStr);
}

static inline void 
U8Str_Clear(u8_str* Dest) {
    Dest->Count = 0;
}

static inline b32
U8Str_PushU32(u8_str* Dest, u32 Num) {
    if (Num == 0) {
        U8Str_Push(Dest, '0');
        return True;
    }
    u32 StartPoint = Dest->Count; 
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        b32 Success = U8Str_Push(Dest, (u8)(DigitToConvert + '0'));
        if (!Success) {
            return False;
        }
    }
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, 
             Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Count - 1 - I]);
    }
    return True;
}

static inline b32
U8Str_PushS32(u8_str* Dest, i32 Num) {
    if (Num == 0) {
        if(!U8Str_Push(Dest, '0')) {
            return False;
        }
        return True;
    }
    
    u32 StartPoint = Dest->Count; 
    
    b32 Negative = Num < 0;
    Num = AbsOf(Num);
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
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

//~ NOTE(Momo): strings with fixed buffer
template<u32 Cap>
struct u8_fstr {
    u8 Data[Cap];
    u32 Count;
};

template<u32 Cap>
static inline void 
U8FStr_Clear(u8_fstr<Cap>* S) {
    S->Count = 0;
}

template<u32 Cap>
static inline u32
U8FStr_Remaining(u8_fstr<Cap> S) {
    return Cap - S.Count;
}

template<u32 Cap>
static inline u8_cstr
U8FStr_ToCStr(u8_fstr<Cap> S) {
    u8_cstr Ret = {};
    Ret.Data = S.Data;
    Ret.Count = S.Count;
    return Ret;
}

template<u32 Cap>
static inline u8_cstr
U8FStr_ToCStr(u8_fstr<Cap>* S) {
    u8_cstr Ret = {};
    Ret.Data = S->Data;
    Ret.Count = S->Count;
    return Ret;
}


template<u32 Cap>
static inline u8*
U8FStr_Push(u8_fstr<Cap>* Dest, u8 Item) {
    if (Dest->Count < Cap) {
        u8* Ret = Dest->Data + Dest->Count++;
        (*Ret) = Item;
        return Ret;
    } 
    else {
        return Null;
    }
}


template<u32 Cap>
static inline b32
U8FStr_PushU32(u8_fstr<Cap>* Dest, u32 Num) {
    if (Num == 0) {
        if(!U8FStr_Push(Dest, '0')) {
            return False;
        }
        return True;
    }
    u32 StartPoint = Dest->Count; 
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        if(!U8FStr_Push(Dest, (u8)(DigitToConvert + '0'))){
            return False;
        }
    }
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, 
             Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Count - 1 - I]);
    }
    
    return True;
}

template<u32 Cap>
static inline b32
U8FStr_PushS32(u8_fstr<Cap>* Dest, i32 Num) {
    if (Num == 0) {
        if (!U8FStr_Push(Dest, '0')) {
            return False;
        }
        return True;
    }
    
    u32 StartPoint = Dest->Count; 
    
    b32 Negative = Num < 0;
    Num = AbsOf(Num);
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        if (!U8FStr_Push(Dest, (char)(DigitToConvert + '0'))) {
            return False;
        }
    }
    
    if (Negative) {
        if (!U8FStr_Push(Dest, '-')) {
            return False;
        }
    }
    
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Count-1-I]);
        
    }
}

template<u32 Cap>
static inline b32
U8FStr_CopyCStr(u8_fstr<Cap>* Dest, u8_cstr Src) {
    if (Src.Count > Cap) {
        return False;
    }
    for (u32 I = 0; I < Src.Count; ++I ) {
        Dest->Data[I] = Src.Data[I];
    }
    Dest->Count = Src.Count;
    return True;
}


template<u32 Cap, u32 Cap2>
static inline b32
U8FStr_Copy(u8_fstr<Cap>* Dest, u8_fstr<Cap2>* Src) {
    return U8FStr_CopyCStr(Dest, U8FStr_ToCStr(Src));
}

template<u32 Cap>
static inline b32
U8FStr_Pop(u8_fstr<Cap>* S) {
    if (S->Count <= 0) {
        return False;
    }
    --S->Count;
    return True;
}

template<u32 Cap>
static inline b32
U8FStr_PushCStr(u8_fstr<Cap>* Dest, u8_cstr Src) {
    if (Dest->Count + Src.Count <= Cap) {
        for ( u32 I = 0; I < Src.Count; ++I ) {
            Dest->Data[Dest->Count++] = Src.Data[I];
        }
        return True;
    }
    return False;
}

template<u32 Cap, u32 Cap2>
static inline b32
U8FStr_PushFStr(u8_fstr<Cap>* Dest, u8_fstr<Cap2>* Src) {
    return U8FStr_PushCStr(Dest, U8FStr_ToCStr(Src));
}

#endif
