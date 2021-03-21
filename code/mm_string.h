#ifndef MM_STRING
#define MM_STRING

//~ NOTE(Momo): u8_cstr

// TODO(Momo): Support for UTF8??
struct u8_cstr {
    u8* Data;
    u32 Size;
};

static inline u8_cstr
U8CStr_Create(u8* Data, u32 Size) {
    u8_cstr Ret = {};
    Ret.Data = Data;
    Ret.Size = Size;
    
    return Ret;
}

// Assumes C-String
static inline u8_cstr
U8CStr_FromSiStr(const char* SiStr) {
    return U8CStr_Create((u8*)SiStr, SiStrLen(SiStr));
}

static inline b32
U8CStr_Compare(u8_cstr Lhs, u8_cstr Rhs) {
    if(Lhs.Size != Rhs.Size) {
        return FALSE;
    }
    for (u32 I = 0; I < Lhs.Size; ++I) {
        if (Lhs.Data[I] != Rhs.Data[I]) {
            return FALSE;
        }
    }
    return TRUE;
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
    for(u32 I = StartIndex; I < Str.Size; ++I) {
        if(Str.Data[I] == Item) {
            return I;
        }
    }
    return Str.Size;
}

static inline u8_cstr_split_res
U8CStr_SplitByDelimiter(u8_cstr Str, arena* Arena, u8 Delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<string> struct.
    u8_cstr_split_res Ret = {};
    u32 Min = 0;
    u32 Max = 0;
    
    for (;Max != Str.Size;) {
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
        u8_cstr Str;
        struct {
            u8* Data;
            u32 Size;
        };
    };
    u32 Cap;
};

static inline u8_str
U8Str_Create(u8* Buffer, u32 Capacity) {
    u8_str Ret = {};
    Ret.Data = Buffer;
    Ret.Size = 0;
    Ret.Cap = Capacity;
    return Ret;
}

static inline u8_str
U8Str_CreateFromArena(arena* Arena, u32 Capacity) {
    u8_str Ret = {};
    u8* Buffer = Arena_PushArray(u8, Arena, Capacity);
    return U8Str_Create(Buffer, Capacity);
}

static inline void
U8Str_Pop(u8_str* S) {
    Assert(S->Size > 0);
    --S->Size;
}

static inline u32 
U8Str_Remaining(u8_str* Buffer) {
    return Buffer->Cap - Buffer->Size;
}

static inline void
U8Str_Copy(u8_str* Dest, u8_cstr Src) {
    Assert(Src.Size <= Dest->Cap);
    for (u32 I = 0; I < Src.Size; ++I ) {
        Dest->Data[I] = Src.Data[I];
    }
    Dest->Size = Src.Size;
}

static inline void
U8Str_NullTerm(u8_str* Dest) {
    Assert(Dest->Size < Dest->Cap);
    Dest->Data[Dest->Size] = 0;
}

static inline void
U8Str_Push(u8_str* Dest, u8 Item) {
    Assert(Dest->Size < Dest->Cap); 
    Dest->Data[Dest->Size++] = Item;
}

static inline void
U8Str_PushCStr(u8_str* Dest, u8_cstr Src) {
    Assert(Dest->Size + Src.Size <= Dest->Cap);
    for ( u32 I = 0; I < Src.Size; ++I ) {
        Dest->Data[Dest->Size++] = Src.Data[I];
    }
}

static inline void
U8Str_PushStr(u8_str* Dest, u8_str* Src) {
    U8Str_PushCStr(Dest, Src->Str);
}

static inline void 
U8Str_Clear(u8_str* Dest) {
    Dest->Size = 0;
}

static inline void 
U8Str_PushU32(u8_str* Dest, u32 Num) {
    if (Num == 0) {
        U8Str_Push(Dest, '0');
        return;
    }
    u32 StartPoint = Dest->Size; 
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        U8Str_Push(Dest, (u8)(DigitToConvert + '0'));
    }
    
    // Reverse starting from start point to count
    u32 SubStrLenHalved = (Dest->Size - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, 
             Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Size - 1 - I]);
    }
}

static inline void 
U8Str_PushI32(u8_str* Dest, i32 Num) {
    if (Num == 0) {
        U8Str_Push(Dest, '0');
        return;
    }
    
    u32 StartPoint = Dest->Size; 
    
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
    u32 SubStrLenHalved = (Dest->Size - StartPoint)/2;
    for(u32 I = 0; I < SubStrLenHalved; ++I) {
        Swap(u8, Dest->Data[StartPoint + I], 
             Dest->Data[Dest->Size-1-I]);
        
    }
}


#endif
