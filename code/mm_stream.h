#ifndef __MM_STREAM__
#define __MM_STREAM__

#include "mm_array.h"

// TODO: Linked list of stream chunks?
struct Stream {
    Array<u8> contents;
    usize current;

    // For bit reading
    u32 bit_buffer;
    u32 bit_count;


    b8 is_eos() const;
    void* read(usize amount);
    u32 read_bits(u32 amount);
    b8 write(void* src, usize src_size);

    template<typename T> 
    T* read();
    
    template<typename T>
    b8 write(T item);

    static Stream create(void* memory, usize size);
    static Stream create(Memory_Arena* arena, usize capacity);
};

template<typename T> 
T* Stream::read() {
    return (T*)this->read(sizeof(T));
}
template<typename T>
b8 Stream::write(T item){
    return this->write(&item, sizeof(T));
}

#endif
