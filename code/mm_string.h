#ifndef MM_STRING
#define MM_STRING

typedef array<char> string;
typedef list<char> string_buffer;

static inline string_buffer
CreateStringBuffer(char* Memory, usize Capacity) {
    return CreateList<char>(Memory, Capacity);
}

static inline string_buffer
CreateStringBuffer(arena* Arena, usize Capacity) {
    return CreateList<char>(Arena, Capacity);
}

static inline string
CreateString(char* Elements, usize Count) {
    return CreateArray(Elements, Count);
}

// Assumes C-String
static inline string 
CreateString(char* SiStr) {
    return CreateString(SiStr, SiStrLen(SiStr));
}

// Assumes C-String
static inline string 
CreateString(const char* SiStr) {
    return CreateString((char*)SiStr);
}

static inline string
SubString(string Src, range<usize> Range) {
    return SubArray(Src, Range);
}

static inline array<string>
DelimitSplit(string Str, arena* Arena, char Delimiter) {
    // We are having faith that the arena given is a bump arena.
    // i.e. strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<string> struct.
    array<string> Ret ={}; 
    range<usize> Range = {};
    
    for (;Range.End != Str.Count;) {
        Range.End = Find(&Str, ' ', Range.Start); 
        
        string* Link = PushStruct<string>(Arena);
        Assert(Link);
        (*Link) = SubString(Str, Range);
        
        if (Ret.Elements == nullptr) {
            Ret.Elements = Link;            
        }
        
        Range.Start = Range.End + 1;
        ++Ret.Count;
    }
    return Ret;
}

static inline void
NullTerm(string_buffer* Dest) {
    Assert(Dest->Count < Dest->Capacity);
    Dest->Elements[Dest->Count] = 0;
}

static inline void 
PushU32(string_buffer* Dest, u32 Num) {
    if (Num == 0) {
        Push(Dest, '0');
        return;
    }
    
    usize StartPoint = Dest->Count; 
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        Push(Dest, (char)(DigitToConvert + '0'));
    }
    
    // Reverse starting from start point to count
    usize SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(usize I = 0; I < SubStrLenHalved; ++I) {
        Swap(&Dest->Elements[StartPoint + I], 
             &Dest->Elements[Dest->Count-1-I]);
    }
}

static inline void 
PushI32(string_buffer* Dest, i32 Num) {
    if (Num == 0) {
        Push(Dest, '0');
        return;
    }
    
    usize StartPoint = Dest->Count; 
    
    b32 Negative = Num < 0;
    Num = Abs(Num);
    
    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        Push(Dest, (char)(DigitToConvert + '0'));
    }
    
    if (Negative) {
        Push(Dest, '-');
    }
    
    // Reverse starting from start point to count
    usize SubStrLenHalved = (Dest->Count - StartPoint)/2;
    for(usize I = 0; I < SubStrLenHalved; ++I) {
        Swap(&Dest->Elements[StartPoint + I], 
             &Dest->Elements[Dest->Count-1-I]);
    }
}


#endif
