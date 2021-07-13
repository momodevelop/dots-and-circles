/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename T>
struct Array {
    T* data;
    umi count;
    
    auto& operator[](umi index);
    T* operator+(umi index);
    
    b8 init(T* buffer, umi cap);
    b8 alloc(Arena* arena, umi count);
    T* get(umi index);
};

#include "momo_array.cpp"
#endif //MM_ARRAY_H
