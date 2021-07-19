/* date = June 27th 2021 1:31 pm */

#ifndef MOMO_CIRCLE_H
#define MOMO_CIRCLE_H
//~ NOTE(Momo): Circles
struct circle2f {
    v2f origin;
    f32 radius;
    
};

static inline circle2f
circle2f_Create(v2f origin, f32 radius) {
    return { origin, radius };
}

static inline circle2f
Translate(circle2f lhs, v2f v) {
    lhs.origin = lhs.origin + v;
    return lhs;
}


#endif //MOMO_CIRCLE_H
