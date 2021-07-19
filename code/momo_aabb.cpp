
//~ NOTE(Momo): aabb2u
static inline aabb2u
aabb2u_Create(v2u min, v2u max) {
    return { min, max };
}

static inline aabb2u
aabb2u_Create(u32 min_x, u32 min_y, u32 max_x, u32 max_y) {
    v2u min = { min_x, min_y };
    v2u max = { max_x, max_y };
    
    return aabb2u_Create(min, max);
    
}


static inline u32
Width(aabb2u a) {
    return a.max.x - a.min.x;
}

static inline u32
Height(aabb2u a) {
    return a.max.y - a.min.y; 
}

static inline v2u
Dimensions(aabb2u a) {
    return { Width(a), Height(a) };
}

static inline aabb2f
Mul(aabb2f lhs, f32 rhs) {
    lhs.min = Mul(lhs.min, rhs);
    lhs.max = Mul(lhs.max, rhs);
    return lhs;
}


static inline aabb2u
aabb2u_CreateXYWH(u32 x, u32 y, u32 w, u32 h) {
    aabb2u ret = {};
    ret.min.x = x;
    ret.min.y = y;
    ret.max.x = x + w;
    ret.max.y = y + h;
    return ret;
}

static inline aabb2u
aabb2u_CreateWH(u32 w, u32 h) {
    aabb2u ret = {};
    ret.max.x = w;
    ret.max.y = h;
    
    return ret;
}

//~ NOTE(Momo): aabb2f
static inline f32
Width(aabb2f a) {
    return a.max.x - a.min.x;
}

static inline f32
Height(aabb2f a) {
    return a.max.y - a.min.y; 
}

static inline f32
Aspect(aabb2f a) {
    return Width(a)/Height(a);
}


static inline aabb3f
aabb2u_CreateCentered(v3f dimensions, v3f anchor) {
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
aabb2f_Create(aabb2i v) {
    aabb2f ret = {};
    ret.min = v2f_Create(v.min);
    ret.max = v2f_Create(v.max);
    return ret;
}

static inline aabb2f
aabb2f_Create(aabb2u v) {
    aabb2f Ret = {};
    Ret.min = v2f_Create(v.min);
    Ret.max = v2f_Create(v.max);
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
    return Ratio(aabb2f_Create(a), aabb2f_Create(b));
}

static inline aabb2u
Translate(aabb2u aabb, u32 x, u32 y) {
    aabb2u ret = aabb;
    ret.min.x += x;
    ret.min.y += y;
    ret.max.x += x;
    ret.max.y += y;
    return ret;
}

