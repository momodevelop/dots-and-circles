/* date = May 22nd 2021 3:34 pm */

#ifndef MOMO_LIST_H
#define MOMO_LIST_H

template<typename T>
struct List;

template<typename T>
struct List_Forward_Itr {
    List<T>* list;
    umi index;
    
    T& operator->();
    List_Forward_Itr<T>& operator++();
    T& operator*();
    b8 operator!=(List_Forward_Itr<T> rhs);
    
};

template<typename T>
struct List_Reverse_Itr {
    List<T>* list;
    umi index;
    
    T& operator->();
    List_Reverse_Itr<T>& operator++();
    T& operator*();
    b8 operator!=(List_Reverse_Itr<T> rhs);
    
};


template<typename T>
struct List {
    T* data;
    umi count;
    umi cap;
    
    T& operator[](umi index);
    T* operator+(umi index);
    
    b8 init(T* data, umi cap);
    b8 alloc(Arena* arena, umi cap);
    void clear();
    T* push();
    T* push_item(T item);
    b8 slear(umi index);
    b8 pop();
    T* last();
    umi remaining();
    T* get(umi index);
    
    template<typename Callback, typename ...Args> 
        void foreach(Callback cb, Args...args);
    template<typename Callback, typename ...Args>
        void foreach_slear_if(Callback cb, Args... arg);
    
    // iterators
    List_Forward_Itr<T> begin();
    List_Forward_Itr<T> end();
    List_Reverse_Itr<T> rbegin();
    List_Reverse_Itr<T> rend();
    
    
};

#include "momo_list.cpp"

#endif //MOMO_LIST_H
