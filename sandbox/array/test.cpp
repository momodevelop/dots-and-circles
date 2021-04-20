#include <stdio.h>

template<typename type>
struct array {
    type* Data;
    int Count;
};

template<typename type, u32 Cap>
struct farray {
    type Data[Cap];
}

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


