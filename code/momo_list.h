/* date = May 22nd 2021 3:34 pm */

#ifndef MOMO_LIST_H
#define MOMO_LIST_H

template<typename T>
struct List {
    T* data;
    u32 count;
    u32 cap;
    
    inline T& operator[](u32 index);
};

#include "momo_list.cpp"

#endif //MOMO_LIST_H
