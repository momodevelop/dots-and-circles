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
    
};

struct v2s {
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
};

struct v3s {
    union {
        s32 elements[3];
        struct {
            union {
                v2s xy;
                struct {
                    s32 x, y;
                };
            };
            s32 z;
        };
        
        struct {
            union {
                v2s wh;
                struct {
                    s32 w, h;
                };
            };
            s32 d;
        }; 
    };
    
    inline s32& operator[](u32 i);
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
    
};


//~ NOTE(Momo): v2f functions
f32&
v2f::operator[](u32 i) {
    ASSERT(i < 2); 
    return elements[i]; 
}

v2f 
v2f_Create(f32 x, f32 y)  {
    return { x, y };
}

static inline v2f 
Add(v2f l, v2f r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

static inline v2f 
operator+(v2f lhs, v2f rhs) {
    return Add(lhs, rhs);
}

static inline v2f&
operator+=(v2f& lhs, v2f rhs) {
    return lhs = lhs + rhs;
}


static inline v2f
Sub(v2f l, v2f r) {
    l.x -= r.x;
    l.y -= r.y;
    return l;
}


static inline v2f 
operator-(v2f lhs, v2f rhs) {
    return Sub(lhs, rhs);
}

static inline v2f&
operator-=(v2f& lhs, v2f rhs) {
    return lhs = lhs - rhs;
}

static inline v2f
Mul(v2f l, f32 r) {
    l.x *= r;
    l.y *= r;
    return l;
}


static inline v2f 
operator*(v2f lhs, f32 rhs) {
    return Mul(lhs, rhs);
}

static inline v2f 
operator*=(v2f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v2f 
operator*(f32 lhs, v2f rhs) {
    return Mul(rhs, lhs);
}


static inline v2f
Div(v2f l, f32 r) {
    l.x /= r;
    l.y /= r;
    
    return l;
}

static inline v2f
Ratio(v2f l, v2f r) {
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
IsEqual(v2f l, v2f r) {
    return 
        IsEqual(l.x, r.x) && 
        IsEqual(l.y, r.y);
}

static inline f32 
Dot(v2f l, v2f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y;
    return ret;
}

static inline v2f 
Midpoint(v2f l, v2f r)  {
    return (l + r) * 0.5f; 
}

static inline f32
LengthSq(v2f v) { 
    // NOTE(Momo): Dot Product trick!
    return Dot(v, v);
}


static inline f32 
Length(v2f l)  { 
    return Sqrt(LengthSq(l));
}

static inline f32
DistanceSq(v2f l, v2f r) {
    return LengthSq(r - l);
}

static inline f32
Distance(v2f l, v2f r)  { 
    return Sqrt(DistanceSq(l, r)); 
}

static inline v2f 
Normalize(v2f V)  {
    f32 len = Length(V);
    v2f ret = Div(V, len);
    return ret;
}

static inline f32
AngleBetween(v2f l, v2f r) {
    f32 l_len = Length(l);
    f32 r_len = Length(r);
    f32 lr_dot = Dot(l,r);
    f32 ret = Acos(lr_dot/(l_len * r_len));
    return ret;
}

static inline b8
IsPerpendicular(v2f l, v2f r) { 
    f32 lr_dot = Dot(l,r);
    return IsEqual(lr_dot, 0.f); 
}


static inline b8 
IsSameDirection(v2f l, v2f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
IsOppositeDirection(v2f l, v2f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot < 0;
}

static inline v2f 
Project(v2f from, v2f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = LengthSq(to);
    
    f32 to_dot_from = Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v2f ret = to * unit_projection_scalar;
    return ret;
}

static inline v2f
Rotate(v2f v, f32 rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    
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
v2u_Create(u32 x = 0.f, u32 y = 0.f)  {
    return { x, y };
}

//~ NOTE(Momo): v2s Functions
static inline v2s 
Add(v2s l, v2s r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

static inline v2s
operator+(v2s l, v2s r) {
    return Add(l,r);
}

static inline v2s
Sub(v2s l, v2s r) {
    l.x -= r.x;
    l.y -= r.y;
    return l;
}


static inline v2s
operator-(v2s l, v2s r) {
    return Sub(l,r);
}

static inline v2s
Mul(v2s l, f32 r) {
    l.x = s32(l.x * r);
    l.y = s32(l.y * r);
    return l;
}


static inline v2s 
operator*(v2s lhs, f32 rhs) {
    return Mul(lhs, rhs);
}

//~ NOTE(Momo): v3s Functions
s32&
v3s::operator[](u32 i) {
    ASSERT(i < 3); 
    return elements[i]; 
}

v3s 
v3s_Create(s32 x = 0, s32 y = 0, s32 z = 0)  {
    return { x, y, z };
}

v3s
Cross(v3s lhs, v3s rhs) {
    return {
        (lhs.y * rhs.z) - (lhs.z * rhs.y),
        (lhs.z * rhs.x) - (lhs.x * rhs.z),
        (lhs.x * rhs.y) - (lhs.y * rhs.x)
    };
}


//~ NOTE(Momo): v3f Functions
f32&
v3f::operator[](u32 i) {
    ASSERT(i < 3); 
    return elements[i]; 
}

v3f 
v3f_Create(f32 x = 0, f32 y = 0, f32 z = 0)  {
    return { x, y, z };
}

static inline v3f 
Add(v3f l, v3f r) {
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    return l;
}

static inline v3f
operator+(v3f l, v3f r) {
    return Add(l,r);
}

static inline v3f&
operator+=(v3f& l, v3f r) {
    return l = l + r;
}


static inline v3f
Sub(v3f l, v3f r) {
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    return l;
}


static inline v3f
operator-(v3f l, v3f r) {
    return Sub(l,r);
}

static inline v3f&
operator-=(v3f& l, v3f r) {
    return l = l - r;
}



static inline v3f
Mul(v3f l, f32 r) {
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}


static inline v3f 
operator*(v3f lhs, f32 rhs) {
    return Mul(lhs, rhs);
}

static inline v3f 
operator*=(v3f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v3f 
operator*(f32 lhs, v3f rhs) {
    return Mul(rhs, lhs);
}


static inline v3f
Div(v3f l, f32 r) {
    ASSERT(!IsEqual(r, 0));
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
IsEqual(v3f l, v3f r) {
    return 
        IsEqual(l.x, r.x) && 
        IsEqual(l.y, r.y) &&
        IsEqual(l.z, r.z);
}

static inline f32 
Dot(v3f l, v3f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y + l.z * r.z;
    return ret;
}

static inline v3f 
Midpoint(v3f l, v3f r)  { 
    return (l + r) * 0.5f; 
}

static inline f32
LengthSq(v3f V) { 
    // NOTE(Momo): Dot Product trick!
    return Dot(V, V);
}

static inline f32
DistanceSq(v3f l, v3f r) {
    f32 ret = LengthSq(l - r); 
    return ret;
}

static inline f32
Distance(v3f l, v3f r)  { 
    return Sqrt(DistanceSq(l, r)); 
}

static inline f32 
Length(v3f l)  { 
    return Sqrt(LengthSq(l));
}

static inline v3f 
Normalize(v3f v)  {
    f32 len = Length(v);
    v3f ret = Div(v, len);
    return ret;
}

static inline f32
AngleBetween(v3f L, v3f R) {
    f32 l_len = Length(L);
    f32 r_len = Length(R);
    f32 LRDot = Dot(L,R);
    f32 Ret = Acos(LRDot/(l_len * r_len));
    
    return Ret;
}


static inline b8
IsPerpendicular(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return IsEqual(lr_dot, 0); 
}


static inline b8 
IsSameDirection(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
IsOppositeDirection(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot < 0;
}

static inline v3f 
Project(v3f from, v3f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = LengthSq(to);
    
    f32 to_dot_from = Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v3f ret = Mul(to, unit_projection_scalar);
    return ret;
}


//~ NOTE(Momo): v4f 
f32&
v4f::operator[](u32 i) {
    ASSERT(i < 4); 
    return elements[i]; 
}

v4f
v4f_Create(f32 x = 0, f32 y = 0, f32 z = 0, f32 w = 0)  {
    return { x, y, z, w };
}


//~ TODO: I'm not sure if these are better are free functions or member functions...
static inline v3f 
v3f_Create(v2f v) {
    v3f ret = {};
    ret.x = v.x;
    ret.y = v.y;
    ret.z = 0.f;
    
    return ret;
}

static inline v2f 
v2f_Create(v2s v) {
    return { f32(v.x), f32(v.y) };
}

static inline v2f
v2f_Create(v2u v) {
    return { f32(v.x), f32(v.y) };
}

static inline v2s 
v2s_Create(v2f v) {
    return { s32(v.x), s32(v.y) };
}


#endif //MOMO_VECTOR_H
