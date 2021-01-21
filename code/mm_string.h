#ifndef MM_STRING
#define MM_STRING

#include "mm_array.h"
#include "mm_list.h"
#include "mm_link_list.h"
#include "mm_arena.h"

using string = array<char>;
using string_buffer = list<char>;


static inline string_buffer
StringBuffer(char* Memory, usize Capacity) {
    return List<char>(Memory, Capacity);
}

static inline string_buffer
StringBuffer(arena* Arena, usize Capacity) {
    return List<char>(Arena, Capacity);
}


static inline string
String(char* Elements, usize Count) {
    return Array<char>(Elements, Count);
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
        Range.End = Find(Str, ' ', Range.Start); 
        

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
    usize SubStrLen = Dest->Count - StartPoint;
    for(usize I = 0; I < SubStrLen/2; ++I) {
        Swap(Dest->Elements[StartPoint + I], Dest->Elements[Dest->Count-1-I]);
    }
}


#endif
