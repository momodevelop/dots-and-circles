/* date = May 22nd 2021 2:37 pm */

#ifndef MOMO_QUEUE_H
#define MOMO_QUEUE_H

template<typename T>
struct Queue {
    T* data;
    u32 cap;
    u32 start;
    u32 finish;
    
    inline b8 init(T* buffer, u32 cap);
    inline b8 alloc(Arena* arena, u32 cap);
    inline b8 is_empty();
    inline b8 is_full();
    inline T* push();
    inline T* next();
    inline b8 pop();
    inline T* get(u32 index);
    
    template<typename Callback, typename... Args> 
        inline void _foreach_sub(u32 start, u32 finish, Callback callback, Args... args);
    
    template<typename Callback, typename... Args> 
        inline void foreach(Callback callback, Args... args);
    
    template<typename Callback, typename... Args>
        inline void pop_until(Callback callback, Args... args);
};

#include "momo_queue.cpp"

#endif //MOMO_QUEUE_H
