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

// Maybe this can be moved to Array?
static inline dlink_list<string>
DelimitSplit(string Str, arena* Arena, char Delimiter) {
    dlink_list<string> Ret ={}; 
    range<usize> Range = {};

    while (Range.End != Str.Count) {
        Range.End = Find(Str, ' ', Range.Start); 

        auto* Link = PushCtr<dlink<string>>(Arena, DLink<string>, SubString(Str, Range));
        PushBack(&Ret, Link);
 
        Range.Start = Range.End + 1;
    }
    return Ret;
}

static inline void
NullTerm(string_buffer* Dest) {
    Assert(Dest->Count < Dest->Capacity);
    Dest->Elements[Dest->Count] = 0;
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

    Reverse(&Dest->Array);
}


#endif
