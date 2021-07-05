/* date = June 27th 2021 0:22 pm */

#ifndef MOMO_VECTOR_H
#define MOMO_VECTOR_H
//~ NOTE(Momo): Vectors
struct v2f {
    union {
        f32 elements[2];
        struct {
            f32 x;
            f32 y;
        };
        struct {
            f32 u;
            f32 v;
        };
        struct {
            f32 w;
            f32 h;
        };
    };
    
    inline f32& operator[](u32 i);
    static inline v2f create(f32 x = 0.f, f32 y = 0.f);
};

struct v2u {
    union {
        u32 elements[2];
        struct {
            u32 x;
            u32 y;
        };
        struct {
            u32 w;
            u32 h;
        };
    };
    
    inline u32& operator[](u32 i);
    static inline v2u create(u32 x = 0, u32 y = 0);
    
};

struct v2i {
    union {
        s32 elements[2];
        struct {
            s32 x;
            s32 y;
        };
        struct {
            s32 w;
            s32 h;
        };
    };
    inline auto& operator[](u32 i) { 
        ASSERT(i < 2); 
        return elements[i]; 
    }
};

struct v3f {
    union {
        f32 elements[3];
        struct {
            union {
                v2f xy;
                struct {
                    f32 x, y;
                };
            };
            f32 z;
        };
        
        struct {
            union {
                v2f wh;
                struct {
                    f32 w, h;
                };
            };
            f32 d;
        }; 
    };
    
    inline f32& operator[](u32 i);
    static inline v3f create(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f);
};

struct v4f {
    union {
        f32 elements[4];
        struct {
            union {
                v3f xyz;
                struct {
                    f32 x, y, z;
                };
                struct {
                    union {
                        v2f xy;
                    };
                    f32 z;
                };
            }; 
            f32 w;
        };
        
        
    };
    
    inline f32& operator[](u32 i);
    static inline v4f create(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f, f32 w = 0.f);
    
};


//~ NOTE(Momo): v2f functions
f32&
v2f::operator[](u32 i) {
    ASSERT(i < 2); 
    return elements[i]; 
}

v2f 
v2f::create(f32 x, f32 y)  {
    return { x, y };
}

static inline v2f 
add(v2f l, v2f r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

static inline v2f 
operator+(v2f lhs, v2f rhs) {
    return add(lhs, rhs);
}

static inline v2f&
operator+=(v2f& lhs, v2f rhs) {
    return lhs = lhs + rhs;
}


static inline v2f
sub(v2f l, v2f r) {
    l.x -= r.x;
    l.y -= r.y;
    return l;
}


static inline v2f 
operator-(v2f lhs, v2f rhs) {
    return sub(lhs, rhs);
}

static inline v2f&
operator-=(v2f& lhs, v2f rhs) {
    return lhs = lhs - rhs;
}

static inline v2f
mul(v2f l, f32 r) {
    l.x *= r;
    l.y *= r;
    return l;
}


static inline v2f 
operator*(v2f lhs, f32 rhs) {
    return mul(lhs, rhs);
}

static inline v2f 
operator*=(v2f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v2f 
operator*(f32 lhs, v2f rhs) {
    return mul(rhs, lhs);
}


static inline v2f
div(v2f l, f32 r) {
    l.x /= r;
    l.y /= r;
    
    return l;
}

static inline v2f
ratio(v2f l, v2f r) {
    l.x /= r.x;
    l.y /= r.y;
    
    return l;
}

static inline v2f
neg(v2f v){
    v.x = -v.x;
    v.y = -v.y;
    return v;
}

static inline b8 
is_equal(v2f l, v2f r) {
    return 
        is_equal(l.x, r.x) && 
        is_equal(l.y, r.y);
}

static inline f32 
dot(v2f l, v2f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y;
    return ret;
}

static inline v2f 
midpoint(v2f l, v2f r)  {
    return (l + r) * 0.5f; 
}

static inline f32
length_sq(v2f v) { 
    // NOTE(Momo): Dot Product trick!
    return dot(v, v);
}


static inline f32 
length(v2f l)  { 
    return sqrt(length_sq(l));
}

static inline f32
distance_sq(v2f l, v2f r) {
    return length_sq(r - l);
}

static inline f32
distance(v2f l, v2f r)  { 
    return sqrt(distance_sq(l, r)); 
}

static inline v2f 
normalize(v2f V)  {
    f32 len = length(V);
    v2f ret = div(V, len);
    return ret;
}

static inline f32
angle_between(v2f l, v2f r) {
    f32 l_len = length(l);
    f32 r_len = length(r);
    f32 lr_dot = dot(l,r);
    f32 ret = acos(lr_dot/(l_len * r_len));
    return ret;
}

static inline b8
is_perpendicular(v2f l, v2f r) { 
    f32 lr_dot = dot(l,r);
    return is_equal(lr_dot, 0.f); 
}


static inline b8 
is_same_direction(v2f l, v2f r) { 
    f32 lr_dot = dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
is_opposite_direction(v2f l, v2f r) { 
    f32 lr_dot = dot(l,r);
    return lr_dot < 0;
}

static inline v2f 
project(v2f from, v2f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = length_sq(to);
    
    f32 to_dot_from = dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v2f ret = to * unit_projection_scalar;
    return ret;
}

static inline v2f
rotate(v2f v, f32 rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 c = cos(rad);
    f32 s = sin(rad);
    
    v2f ret = {};
    ret.x = (c * v.x) - (s * v.y);
    ret.y = (s * v.x) + (c * v.y);
    return ret;
}

//~ NOTE(Momo): v2u Functions
u32&
v2u::operator[](u32 i) {
    ASSERT(i < 2); 
    return elements[i]; 
}

v2u 
v2u::create(u32 x, u32 y)  {
    return { x, y };
}

//~ NOTE(Momo): v3f Functions
f32&
v3f::operator[](u32 i) {
    ASSERT(i < 3); 
    return elements[i]; 
}

v3f 
v3f::create(f32 x, f32 y, f32 z)  {
    return { x, y, z };
}

static inline v3f 
add(v3f l, v3f r) {
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    return l;
}

static inline v3f
operator+(v3f l, v3f r) {
    return add(l,r);
}

static inline v3f&
operator+=(v3f& l, v3f r) {
    return l = l + r;
}


static inline v3f
sub(v3f l, v3f r) {
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    return l;
}


static inline v3f
operator-(v3f l, v3f r) {
    return sub(l,r);
}

static inline v3f&
operator-=(v3f& l, v3f r) {
    return l = l - r;
}



static inline v3f
mul(v3f l, f32 r) {
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}


static inline v3f 
operator*(v3f lhs, f32 rhs) {
    return mul(lhs, rhs);
}

static inline v3f 
operator*=(v3f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v3f 
operator*(f32 lhs, v3f rhs) {
    return mul(rhs, lhs);
}


static inline v3f
div(v3f l, f32 r) {
    ASSERT(!is_equal(r, 0));
    l.x /= r;
    l.y /= r;
    l.z /= r;
    return l;
}

static inline v3f
neg(v3f v){
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    return v;
}

static inline b8 
is_equal(v3f l, v3f r) {
    return 
        is_equal(l.x, r.x) && 
        is_equal(l.y, r.y) &&
        is_equal(l.z, r.z);
}

static inline f32 
dot(v3f l, v3f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y + l.z * r.z;
    return ret;
}

static inline v3f 
midpoint(v3f l, v3f r)  { 
    return (l + r) * 0.5f; 
}

static inline f32
length_sq(v3f V) { 
    // NOTE(Momo): Dot Product trick!
    return dot(V, V);
}

static inline f32
distance_sq(v3f l, v3f r) {
    f32 ret = length_sq(l - r); 
    return ret;
}

static inline f32
distance(v3f l, v3f r)  { 
    return sqrt(distance_sq(l, r)); 
}

static inline f32 
length(v3f l)  { 
    return sqrt(length_sq(l));
}

static inline v3f 
normalize(v3f v)  {
    f32 len = length(v);
    v3f ret = div(v, len);
    return ret;
}

static inline f32
angle_between(v3f L, v3f R) {
    f32 l_len = length(L);
    f32 r_len = length(R);
    f32 LRDot = dot(L,R);
    f32 Ret = acos(LRDot/(l_len * r_len));
    
    return Ret;
}


static inline b8
is_perpendicular(v3f l, v3f r) { 
    f32 lr_dot = dot(l,r);
    return is_equal(lr_dot, 0); 
}


static inline b8 
is_same_direction(v3f l, v3f r) { 
    f32 lr_dot = dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
is_opposite_dir(v3f l, v3f r) { 
    f32 lr_dot = dot(l,r);
    return lr_dot < 0;
}

static inline v3f 
project(v3f from, v3f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = length_sq(to);
    
    f32 to_dot_from = dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v3f ret = mul(to, unit_projection_scalar);
    return ret;
}


//~ NOTE(Momo): v4f 
f32&
v4f::operator[](u32 i) {
    ASSERT(i < 4); 
    return elements[i]; 
}

v4f
v4f::create(f32 x, f32 y, f32 z, f32 w)  {
    return { x, y, z, w };
}


//~ TODO: I'm not sure if these are better are free functions or member functions...
static inline v3f 
to_v3f(v2f v) {
    v3f ret = {};
    ret.x = v.x;
    ret.y = v.y;
    ret.z = 0.f;
    
    return ret;
}

static inline v2f 
to_v2f(v2i v) {
    v2f ret = {};
    ret.x = (f32)v.x;
    ret.y = (f32)v.y;return ret;
}

static inline v2f
to_v2f(v2u v) {
    return { f32(v.x), f32(v.y) };
}

static inline v2i 
to_v2i(v2f v) {
    return { s32(v.x), s32(v.y) };
}

#endif //MOMO_VECTOR_H
