#include "mm_stream.h"

Stream Stream::create(void* memory, usize memory_size) {
    Stream Ret = {};
    Ret.contents = array((u8*)memory, memory_size);
    return Ret;
}

Stream Stream::create(Memory_Arena* arena, usize capacity) {
    void* Memory = PushBlock(arena, capacity);
    return Stream::create(Memory, capacity); 
} 

b8 Stream::is_eos() const {
    return this->current >= this->contents.count;
}

void* Stream::read(usize amount) {
    void* ret = nullptr;
    if (this->current + amount <= this->contents.count) {
        ret = this->contents.elements + this->current;
    }
    this->current += amount;
    return ret;
}

// Bits are consumed from LSB to MSB
u32 Stream::read_bits(u32 amount)
{
    Assert(amount <= 32);
    
    while(this->bit_count < amount) {
        u32 byte = *(this->read<u8>());
        this->bit_buffer |= (byte << this->bit_count);
        this->bit_count += 8;
    }

    u32 Result = this->bit_buffer & ((1 << amount) - 1); 

    this->bit_count -= amount;
    this->bit_buffer >>= amount;

    return Result;
}

b8 Stream::write(void* src, usize src_size) {
    if (this->current + src_size >= this->contents.count) {
        return false;
    }
    Copy(this->contents.elements + this->current, src, src_size);
    this->current += src_size; 
    return true;
}

