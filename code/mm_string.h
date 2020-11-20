#ifndef __MOMO_STRING__
#define __MOMO_STRING__

#include "mm_core.h"

// 1. No null terminating value stores
// 2. If the operations overflow, it will cap at the capacity.
// TODO: Is it worth writing a struct to contain 'const' strings such that we do not have to deal with const char* anymore in our code base?



struct string {
    struct {
        usize Length;
        char* Elements;
    };
    inline auto& operator[](usize I) const {
        Assert(I < Length);
        return Elements[I];
    }
};


struct string_it {
    string* String;
    usize Index;

    char* operator->() {
        return &String->Elements[Index];
    }
};

static inline string 
String(char* Elements, usize Length) {
    Assert(Elements != nullptr);
    
    string Ret = {};
    Ret.Elements = Elements;
    Ret.Length = Length;
    
    return Ret;
}


// Assumes C-String
static inline string 
String(char* SiStr) {
    return String(SiStr, SiStrLen(SiStr));
}

// Assumes C-String
static inline string 
String(const char* SiStr) {
    return String((char*)SiStr);
}

static inline string 
String(string Src, range<usize> Slice) {
    return String(Src.Elements + Slice.Start, Slice.End - Slice.Start);
};

struct string_buffer {
    union {
        string String;
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

static inline b32
IsEmpty(string String) {
    return String.Length == 0;
}

static inline string_buffer
StringBuffer(char* Memory, usize Capacity) {
    Assert(Capacity > 0);
    Assert(Memory);

    string_buffer Ret = {};
    Ret.Elements = Memory;
    Ret.Capacity = Capacity;

    return Ret;
}

static inline  usize
Remaining(string_buffer Buffer) {
    return Buffer.Capacity - Buffer.Length;
}

static inline void
Clear(string_buffer* Dest) {
    Dest->Length = 0;
}

static inline void
Copy(string_buffer* Dest, string Src) {
    Assert(Src.Length <= Dest->Capacity);
    for (u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[i] = Src.Elements[i];
    }
    Dest->Length = Src.Length;
}



static inline void
Push(string_buffer* Dest, char Char) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length++] = Char;
}

static inline void
Pop(string_buffer* Dest) {
    Assert(Dest->Length > 0);
    --Dest->Length;
}


static inline void
Concat(string_buffer* Dest, string Src) {
    Assert(Dest->Length + Src.Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[Dest->Length++] = Src.Elements[i];
    }
}


static inline void
NullTerm(string_buffer* Dest) {
    Assert(Dest->Length < Dest->Capacity);
    Dest->Elements[Dest->Length] = 0;
}

static inline b32
Compare(string Lhs, string Rhs) { 
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
Reverse(string_buffer* Dest) {
    for (usize i = 0; i < Dest->Length/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Length-1-i]);
    }
}

static inline void
Itoa(string_buffer* Dest, i32 Num) {
    // Naive method. 
    // Extract each number starting from the back and fill the buffer. 
    // Then reverse it.
    
    // Special case for 0
    if (Num == 0) {
        Push(Dest, '0');
        return;
    }

    b32 Negative = Num < 0;
    Num = Abs(Num);

    for(; Num != 0; Num /= 10) {
        i32 DigitToConvert = Num % 10;
        Push(Dest, (char)(DigitToConvert + '0'));
    }

    if (Negative) {
        Push(Dest, '-');
    }

    Reverse(Dest);
}


static inline usize
Find(string String, char Delimiter, usize From = 0) { 
    for (;;++From) {
         if (From >= String.Length) {
            return String.Length;
         }

         if (String[From] == Delimiter) {
            return From;
         }
    }

    // There shouldn't be a case where this happens?
    Assert(false);
    return From;
}


#include "mm_arena.h"
static inline string_buffer
StringBuffer(arena* Arena, usize Capacity) {
    char* Buffer = PushSiArray<char>(Arena, Capacity); 
    return StringBuffer(Buffer, Capacity);
}

static inline string_buffer* 
PushStringBuffer(arena* Arena, usize Capacity) {
    string_buffer* Ret = PushStruct<string_buffer>(Arena);
    (*Ret) = StringBuffer(Arena, Capacity);
    return Ret;
}



#endif
