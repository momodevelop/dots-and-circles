/* date = June 27th 2021 0:48 pm */

#ifndef MOMO_AABB_H
#define MOMO_AABB_H

struct aabb2i {
    v2i min;
    v2i max;
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
    
    static aabb3f create_centered(v3f dimensions, v3f anchor);
};

//~ NOTE(Momo): aabb2u
inline aabb2u
aabb2u::create(v2u min, v2u max) {
    return { min, max };
}

inline aabb2u
aabb2u::create(u32 min_x, u32 min_y, u32 max_x, u32 max_y) {
    v2u min = v2u::create(min_x, min_y);
    v2u max = v2u::create(max_x, max_y);
    
    return create(min, max);
    
}


static inline u32
width(aabb2u a) {
    return a.max.x - a.min.x;
}

static inline u32
height(aabb2u a) {
    return a.max.y - a.min.y; 
}

static inline v2u
dimensions(aabb2u a) {
    return { width(a), height(a) };
}

static inline aabb2f
mul(aabb2f lhs, f32 rhs) {
    lhs.min = mul(lhs.min, rhs);
    lhs.max = mul(lhs.max, rhs);
    return lhs;
}


inline aabb2u
aabb2u::create_xywh(u32 x, u32 y, u32 w, u32 h) {
    aabb2u ret = {};
    ret.min.x = x;
    ret.min.y = y;
    ret.max.x = x + w;
    ret.max.y = y + h;
    return ret;
}

inline aabb2u
aabb2u::create_wh(u32 w, u32 h) {
    aabb2u ret = {};
    ret.max.x = w;
    ret.max.y = h;
    
    return ret;
}

//~ NOTE(Momo): aabb2f
static inline f32
width(aabb2f a) {
    return a.max.x - a.min.x;
}

static inline f32
height(aabb2f a) {
    return a.max.y - a.min.y; 
}

static inline f32
aspect_ratio(aabb2f a) {
    return width(a)/height(a);
}


inline aabb3f
aabb3f::create_centered(v3f dimensions, v3f anchor) {
    aabb3f ret = {};
    ret.min.x = Lerp(0.f, -dimensions.w, anchor.x);
    ret.max.x = Lerp(dimensions.w, 0.f, anchor.x);
    
    ret.min.y = Lerp(0.f, -dimensions.h, anchor.y);
    ret.max.y = Lerp(dimensions.h, 0.f, anchor.y);
    
    ret.min.z = Lerp(0.f, -dimensions.d, anchor.z);
    ret.max.z = Lerp(dimensions.d, 0.f, anchor.z);
    
    return ret; 
}


static inline aabb2f
to_aabb2f(aabb2i v) {
    aabb2f ret = {};
    ret.min = to_v2f(v.min);
    ret.max = to_v2f(v.max);
    return ret;
}

static inline aabb2f
to_aabb2f(aabb2u v) {
    aabb2f Ret = {};
    Ret.min = to_v2f(v.min);
    Ret.max = to_v2f(v.max);
    return Ret;
}

// NOTE(Momo): Gets the Normalized values of Aabb A based on another Aabb B
static inline aabb2f 
ratio(aabb2f a, aabb2f b) {
    aabb2f ret = {};
    ret.min.x = ratio(a.min.x, b.min.x, b.max.x);
    ret.min.y = ratio(a.min.y, b.min.y, b.max.y);
    ret.max.x = ratio(a.max.x, b.min.x, b.max.x);
    ret.max.y = ratio(a.max.y, b.min.x, b.max.y);
    
    return ret;
}

static inline aabb2f 
ratio(aabb2u a, aabb2u b) {
    return ratio(to_aabb2f(a), to_aabb2f(b));
}

static inline aabb2u
translate(aabb2u aabb, u32 x, u32 y) {
    aabb2u ret = aabb;
    ret.min.x += x;
    ret.min.y += y;
    ret.max.x += x;
    ret.max.y += y;
    return ret;
}

#endif //MOMO_AABB_H
