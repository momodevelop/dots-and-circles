#include <stdio.h>
#include "../../code/mm_core.h"

template<typename type>
struct array {
    type* Data;
    u32 Count;
};

template<typename type>
struct list : array<type> {
    u32 Capacity;
};

// if we want to specialize...
typedef array<u8> u8_cstr;

template<typename type>
static inline array<type>
Array_Create(type* Data, u32 Size) {
    array<type> Ret = {};
    Ret.Data = Data;
    Ret.Size = Size;
    
    return Ret;
}


static inline u8_cstr
U8CStr_FromSiStr(const char* SiStr) {
    
}



int main() {
}


