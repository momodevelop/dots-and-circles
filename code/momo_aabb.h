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
    
    static inline aabb2u create(v2u min, v2u max);
    static inline aabb2u create(u32 min_x, u32 min_y, u32 max_x, u32 max_y);
    static inline aabb2u create_xywh(u32 x, u32 y, u32 w, u32 h);
    static inline aabb2u create_wh(u32 w, u32 h);
};

struct aabb3f {
    v3f min;
    v3f max;
    
    static inline aabb3f create_centered(v3f dimensions, v3f anchor);
};

#include "momo_aabb.cpp"
#endif //MOMO_AABB_H
