/* date = May 22nd 2021 3:34 pm */

#ifndef MOMO_LIST_H
#define MOMO_LIST_H

template<typename T>
struct List;

template<typename T>
struct List_Forward_Itr {
    List<T>* list;
    u32 index;
    
    inline T& operator->();
    inline List_Forward_Itr<T>& operator++();
    inline T& operator*();
    inline b8 operator!=(List_Forward_Itr<T> rhs);
    
};

template<typename T>
struct List_Reverse_Itr {
    List<T>* list;
    u32 index;
    
    inline T& operator->();
    inline List_Reverse_Itr<T>& operator++();
    inline T& operator*();
    inline b8 operator!=(List_Reverse_Itr<T> rhs);
    
};


template<typename T>
struct List {
    T* data;
    u32 count;
    u32 cap;
    
    inline T& operator[](u32 index);
    
    // iterators
    inline List_Forward_Itr<T> begin();
    inline List_Forward_Itr<T> end();
    inline List_Reverse_Itr<T> rbegin();
    inline List_Reverse_Itr<T> rend();
    
    
};

#include "momo_list.cpp"

#endif //MOMO_LIST_H
