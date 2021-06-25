/* date = May 22nd 2021 2:37 pm */

#ifndef MOMO_QUEUE_H
#define MOMO_QUEUE_H

template<typename T>
struct Queue {
    T* data;
    u32 cap;
    u32 start;
    u32 finish;
    
    b8 init(T* buffer, u32 cap);
    b8 alloc(Arena* arena, u32 cap);
    b8 is_empty();
    b8 is_full();
    T* push();
    T* next();
    b8 pop();
    T* get(u32 index);
    
    template<typename Callback, typename... Args> 
        void _foreach_sub(u32 start, u32 finish, Callback callback, Args... args);
    
    template<typename Callback, typename... Args> 
        void foreach(Callback callback, Args... args);
    
    template<typename Callback, typename... Args>
        void pop_until(Callback callback, Args... args);
};

#include "momo_queue.cpp"

#endif //MOMO_QUEUE_H
