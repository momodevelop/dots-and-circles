/* date = May 22nd 2021 3:34 pm */

#ifndef MOMO_LIST_H
#define MOMO_LIST_H

template<typename T>
struct List;

template<typename T>
struct List_Forward_Itr {
    List<T>* list;
    u32 index;
    
    T& operator->();
};

template<typename T>
struct List_Reverse_Itr {
    List<T>* list;
    u32 index;
    
    T& operator->();
    
};


template<typename T>
struct List {
    T* data;
    u32 count;
    u32 cap;
    
    T& operator[](u32 index);
    T* operator+(u32 index);
    
    b8 init(T* data, u32 cap);
    b8 alloc(Arena* arena, u32 cap);
    void clear();
    T* push();
    T* push_item(T item);
    b8 slear(u32 index);
    b8 pop();
    T* last();
    u32 remaining();
    T* get(u32 index);
    
    template<typename Callback, typename ...Args> 
        void foreach(Callback cb, Args...args);
    
    
    // iterators
    List_Forward_Itr<T> begin();
    List_Forward_Itr<T> end();
    List_Reverse_Itr<T> rbegin();
    List_Reverse_Itr<T> rend();
    
    
};

#include "momo_list.cpp"

#endif //MOMO_LIST_H
