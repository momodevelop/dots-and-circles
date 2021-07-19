/* date = June 27th 2021 0:48 pm */

#ifndef MOMO_AABB_H
#define MOMO_AABB_H

struct aabb2i {
    v2s min;
    v2s max;
};

struct aabb2f {
    v2f min;
    v2f max;
};
struct aabb2u {
    v2u min;
    v2u max;
    
};

struct aabb3f {
    v3f min;
    v3f max;
};

#include "momo_aabb.cpp"
#endif //MOMO_AABB_H
