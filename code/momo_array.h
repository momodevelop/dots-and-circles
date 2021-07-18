/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename T>
struct Array {
    T* data;
    u32 count;
    
    inline auto& operator[](u32 index);
};

#include "momo_array.cpp"
#endif //MM_ARRAY_H
