/* date = May 30th 2021 3:47 pm */

#ifndef MOMO_BIGINT_H
#define MOMO_BIGINT_H

// NOTE(Momo): Number is stored in the reverse order
// That is, the number '12345' is stored as '5432100000'.

struct Big_Int;
struct Big_Int_Forward_Itr {
    Big_Int* big_int;
    u32 index;
    
    b8 operator!=(Big_Int_Forward_Itr rhs);
    u8& operator*();
    Big_Int_Forward_Itr& operator++();
};


struct Big_Int_Reverse_Itr {
    Big_Int* big_int;
    u32 index;
    
    b8 operator!=(Big_Int_Reverse_Itr rhs);
    Big_Int_Reverse_Itr& operator++();
    u8& operator*();
};

struct Big_Int {
    u8* data;
    u32 count;
    u32 places;
    
    Big_Int& operator=(u32 rhs);
    Big_Int& operator+=(u32 rhs);
    
    Big_Int_Forward_Itr begin();
    Big_Int_Forward_Itr end();
    Big_Int_Reverse_Itr rbegin();
    Big_Int_Reverse_Itr rend();
    
    void set_zero();
    void set_max();
    
    b8 init(u8* buffer, u32 count);
    b8 alloc(Arena* arena, u32 count);
    void add(u32 value);
    void set(u32 value);
    
};


#include "momo_bigint.cpp"



#endif //MOMO_BIGINT_H
