
//~ NOTE(Momo): aabb2u
aabb2u
aabb2u::create(v2u min, v2u max) {
    return { min, max };
}

aabb2u
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


aabb2u
aabb2u::create_xywh(u32 x, u32 y, u32 w, u32 h) {
    aabb2u ret = {};
    ret.min.x = x;
    ret.min.y = y;
    ret.max.x = x + w;
    ret.max.y = y + h;
    return ret;
}

aabb2u
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


aabb3f
aabb3f::create_centered(v3f dimensions, v3f anchor) {
    aabb3f ret = {};
    ret.min.x = LERP(0.f, -dimensions.w, anchor.x);
    ret.max.x = LERP(dimensions.w, 0.f, anchor.x);
    
    ret.min.y = LERP(0.f, -dimensions.h, anchor.y);
    ret.max.y = LERP(dimensions.h, 0.f, anchor.y);
    
    ret.min.z = LERP(0.f, -dimensions.d, anchor.z);
    ret.max.z = LERP(dimensions.d, 0.f, anchor.z);
    
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
Ratio(aabb2f a, aabb2f b) {
    aabb2f ret = {};
    ret.min.x = Ratio(a.min.x, b.min.x, b.max.x);
    ret.min.y = Ratio(a.min.y, b.min.y, b.max.y);
    ret.max.x = Ratio(a.max.x, b.min.x, b.max.x);
    ret.max.y = Ratio(a.max.y, b.min.x, b.max.y);
    
    return ret;
}

static inline aabb2f 
Ratio(aabb2u a, aabb2u b) {
    return Ratio(to_aabb2f(a), to_aabb2f(b));
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

