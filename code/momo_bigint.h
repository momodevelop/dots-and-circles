/* date = May 30th 2021 3:47 pm */

#ifndef MOMO_BIGINT_H
#define MOMO_BIGINT_H

// NOTE(Momo): Number is stored in the reverse order
// That is, the number '12345' is stored as '5432100000'.

struct BigInt {
    u8* data;
    u32 cap;
    u32 count;
    
};


#include "momo_bigint.cpp"

#endif //MOMO_BIGINT_H
