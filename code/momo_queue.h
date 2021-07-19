/* date = May 22nd 2021 2:37 pm */

#ifndef MOMO_QUEUE_H
#define MOMO_QUEUE_H

template<typename T>
struct Queue {
    T* data;
    u32 cap;
    u32 start;
    u32 finish;
};

#include "momo_queue.cpp"

#endif //MOMO_QUEUE_H
