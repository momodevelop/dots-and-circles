#ifndef __MOMO_MATH__
#define __MOMO_MATH__



// NOTE(Momo): To future self: We wrap the math functions IN CASE we 
// want to have specialized versions for different systems (using SSE etc)
//
// I guess a good rule of thumb is that the GAME CODE should never directly
// use standard library functions IN CASE we cannot use them!
//

// NOTE(Momo): Constants
#define PI 3.14159265358979323846264338327950288f
#define EPSILON 1.19209290E-07f
#define TAU PI * 2.f

// Get the ratio of Value within the range [Min,Max] 
// Return value Will be [0, 1]
static inline f32
ratio(f32 value, f32 min, f32 max) {
    return (value - min)/(max - min); 
}

// NOTE(Momo): Common Functions
static inline b8
is_equal(f32 L, f32 R) {
    return AbsOf(L - R) <= EPSILON;
}

static inline f32 
deg_to_rad(f32 degrees) {
    return degrees * PI / 180.f;
}

static inline f32 
rad_to_deg(f32 radians) {
    return radians * 180.f / PI;
}

static inline f32 
sin(f32 x) {
    return sinf(x);
}

static inline f32 
cos(f32 x) {
    return cosf(x);
}

static inline f32 
tan(f32 x) {
    return tanf(x);
}


static inline f32 
sqrt(f32 x) {
    return sqrtf(x);
}

static inline f32 
asin(f32 x) {
    return asinf(x);
}

static inline f32 
acos(f32 x) {
    return acosf(x);
}

static inline f32 
atan(f32 x) {
    return atanf(x);
}

static inline f32
pow(f32 b, f32 e) {
    return powf(b,e);
}

//~ NOTE(Momo): Vectors
struct v2f {
    union {
        f32 elements[2] = {0};
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
    inline auto& operator[](u32 i) { 
        Assert(i < 2); 
        return elements[i]; 
    }
    
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
    
    inline auto& operator[](u32 i) { 
        Assert(i < 2); 
        return elements[i]; 
    }
    
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
        Assert(i < 2); 
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
    
    inline auto& operator[](u32 i) { 
        Assert(i < 3); 
        return elements[i]; 
    }
    
};

struct v4f {
    union {
        f32 elements[4] = {0};
        struct {
            union {
                v3f xyz;
                struct {
                    f32 x, y, z;
                };
            }; 
            f32 w;
        };
        
        
    };
    
    inline auto& operator[](u32 i) { 
        Assert(i < 4); 
        return elements[i]; 
    }
    
};

//~ NOTE(Momo): v2f functions


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
    Assert(!is_equal(r.x, 0));
    Assert(!is_equal(r.y, 0));
    
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
static inline v2u 
v2u_create(u32 x, u32 y) {
    v2u ret = {};
    ret.x = x;
    ret.y = y;
    
    return ret;
}

//~ NOTE(Momo): v3f Functions
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
    Assert(!is_equal(r, 0));
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

static inline v3f 
v2f_to_v3f(v2f v) {
    v3f ret = {};
    ret.x = v.x;
    ret.y = v.y;
    ret.z = 0.f;
    
    return ret;
}

static inline v2f
v2u_to_v2f(v2u v) {
    v2f ret = {};
    ret.x = (f32)v.x;
    ret.y = (f32)v.y;
    
    return ret;
}

static inline v2f 
v2i_to_v2f(v2i v) {
    v2f ret = {};
    ret.x = (f32)v.x;
    ret.y = (f32)v.y;return ret;
}

static inline v2f
to_v2f(v2u v) {
    return { f32(v.x), f32(v.y) };
}

//~ NOTE(Momo): AABB 
struct aabb2i {
    v2i Min;
    v2i Max;
};

struct aabb2f {
    v2f Min;
    v2f Max;
};

struct aabb2u {
    v2u Min;
    v2u Max;
};

struct aabb3f {
    v3f Min;
    v3f Max;
};


static inline aabb2u
Aabb2u_CreateFromV2u(v2u Min, v2u Max) {
    aabb2u Ret = {};
    Ret.Min = Min;
    Ret.Max = Max;
    
    return Ret;
}

static inline aabb2u
Aabb2u_Create(u32 MinX, u32 MinY, u32 MaxX, u32 MaxY) {
    v2u Min = v2u_create(MinX, MinY);
    v2u Max = v2u_create(MaxX, MaxY);
    
    return Aabb2u_CreateFromV2u(Min, Max);
    
}


static inline u32
Aabb2u_Width(aabb2u A) {
    return A.Max.x - A.Min.x;
}

static inline u32
Aabb2u_Height(aabb2u A) {
    return A.Max.y - A.Min.y; 
}

static inline v2u
Aabb2u_Dimensions(aabb2u A) {
    v2u Ret = {};
    Ret.w = Aabb2u_Width(A);
    Ret.h = Aabb2u_Height(A);
    
    return Ret;
}

static inline aabb2f
Aabb2f_Mul(aabb2f Lhs, f32 Rhs) {
    Lhs.Min = mul(Lhs.Min, Rhs);
    Lhs.Max = mul(Lhs.Max, Rhs);
    return Lhs;
}

static inline f32
Aabb2f_Width(aabb2f A) {
    return A.Max.x - A.Min.x;
}

static inline f32
Aabb2f_Height(aabb2f A) {
    return A.Max.y - A.Min.y; 
}


static inline v2f
Aabb2f_Dimensions(aabb2f A) {
    v2f Ret = {};
    Ret.w = Aabb2f_Width(A);
    Ret.h = Aabb2f_Height(A);
    
    return Ret;
}


static inline aabb2u
Aabb2u_CreateXYWH(u32 x, u32 y, u32 w, u32 h) {
    aabb2u Ret = {};
    Ret.Min.x = x;
    Ret.Min.y = y;
    Ret.Max.x = x + w;
    Ret.Max.y = y + h;
    return Ret;
}

static inline aabb2u
Aabb2u_CreateWH(u32 w, u32 h) {
    aabb2u Ret = {};
    Ret.Max.x = w;
    Ret.Max.y = h;
    
    return Ret;
}


static inline aabb3f
Aabb3f_CreateCentered(v3f dimensions, v3f anchor) {
    aabb3f Ret = {};
    Ret.Min.x = Lerp(0.f, -dimensions.w, anchor.x);
    Ret.Max.x = Lerp(dimensions.w, 0.f, anchor.x);
    
    Ret.Min.y = Lerp(0.f, -dimensions.h, anchor.y);
    Ret.Max.y = Lerp(dimensions.h, 0.f, anchor.y);
    
    Ret.Min.z = Lerp(0.f, -dimensions.d, anchor.z);
    Ret.Max.z = Lerp(dimensions.d, 0.f, anchor.z);
    
    return Ret; 
}

static inline f32
Aabb2f_AspectRatio(aabb2f v) {
    return (f32)Aabb2f_Width(v)/Aabb2f_Height(v);
}

static inline aabb2f
Aabb2i_To_Aabb2f(aabb2i v) {
    aabb2f Ret = {};
    Ret.Min = v2i_to_v2f(v.Min);
    Ret.Max = v2i_to_v2f(v.Max);
    return Ret;
}

static inline aabb2f
Aabb2u_To_Aabb2f(aabb2u v) {
    aabb2f Ret = {};
    Ret.Min = v2u_to_v2f(v.Min);
    Ret.Max = v2u_to_v2f(v.Max);
    return Ret;
}

// NOTE(Momo): Gets the Normalized values of Aabb A based on another Aabb B
static inline aabb2f 
Aabb2f_Ratio(aabb2f a, aabb2f b) {
    aabb2f ret = {};
    ret.Min.x = ratio(a.Min.x, b.Min.x, b.Max.x);
    ret.Min.y = ratio(a.Min.y, b.Min.y, b.Max.y);
    ret.Max.x = ratio(a.Max.x, b.Min.x, b.Max.x);
    ret.Max.y = ratio(a.Max.y, b.Min.x, b.Max.y);
    
    return ret;
}

static inline aabb2f 
Aabb2u_Ratio(aabb2u A, aabb2u B) {
    aabb2f Af = Aabb2u_To_Aabb2f(A);
    aabb2f Bf = Aabb2u_To_Aabb2f(B);
    return Aabb2f_Ratio(Af, Bf);
}

static inline aabb2u
Aabb2u_Translate(aabb2u aabb, u32 x, u32 y) {
    aabb2u ret = aabb;
    ret.Min.x += x;
    ret.Min.y += y;
    ret.Max.x += x;
    ret.Max.y += y;
    
    return ret;
}

//~ NOTE(Momo): Circles
struct circle2f {
    v2f Origin;
    f32 Radius;
};

static inline circle2f
Circle2f_Create(v2f Origin, f32 Radius) {
    circle2f Ret = {};
    Ret.Origin = Origin;
    Ret.Radius = Radius;
    
    return Ret;
}

static inline circle2f
Circle2f_Offset(circle2f Lhs, v2f Offset) {
    Lhs.Origin = Lhs.Origin + Offset;
    return Lhs;
}

//~ NOTE(Momo): 2x2 Matrices
union m22f {
    v2f elements[2];
    inline auto& operator[](u32 i) { 
        Assert(i < 2); 
        return elements[i]; 
    }
    
};

static inline m22f 
M22f_Concat(m22f L, m22f R) {
    m22f Ret = {};
    for (u8 r = 0; r < 2; r++) { 
        for (u8 c = 0; c < 2; c++) { 
            for (u8 i = 0; i < 2; i++) 
                Ret[r][c] += L[r][i] *  R[i][c]; 
        } 
    } 
    return Ret;
}

static inline v2f
M22f_ConcatV2f(m22f L, v2f R) {
    v2f Ret = {};
    Ret[0] = L[0][0] * R[0] + L[0][1] * R[1];
    Ret[1] = L[1][0] * R[0] + L[1][1] * R[1];
    
    return Ret;
}

static inline m22f 
operator*(m22f L, m22f R) {
    return M22f_Concat(L,R);
}

static inline v2f 
operator*(m22f L, v2f R) {
    return M22f_ConcatV2f(L,R);
}

static inline m22f 
M22f_Identity() {
    m22f Ret = {};
    Ret[0][0] = 1.f;
    Ret[1][1] = 1.f;
    
    return Ret;
}


static inline m22f 
M22f_Rotation(f32 Rad) {
    // NOTE(Momo): 
    // c -s
    // s  c
    
    f32 c = cos(Rad);
    f32 s = sin(Rad);
    m22f Ret = {};
    Ret[0][0] = c;
    Ret[0][1] = -s;
    Ret[1][0] = s;
    Ret[1][1] = c;
    
    return Ret;
}

//~ NOTE(Momo): 4x4 Matrices
union m44f {
    v4f elements[4];
    inline auto& operator[](u32 i) { 
        Assert(i < 4); 
        return elements[i]; 
    }
    
};

static inline m44f 
M44f_Concat(m44f L, m44f R) {
    m44f Ret = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                Ret[r][c] += L[r][i] *  R[i][c]; 
        } 
    } 
    return Ret;
}

static inline m44f 
operator*(m44f L, m44f R) {
    return M44f_Concat(L,R);
}

static inline m44f 
M44f_Identity() {
    m44f Ret = {};
    Ret[0][0] = 1.f;
    Ret[1][1] = 1.f;
    Ret[2][2] = 1.f;
    Ret[3][3] = 1.f;
    
    return Ret;
}

static inline m44f 
M44f_Transpose(m44f M) {
    m44f Ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            Ret[i][j] = M[j][i];
        }
    }
    
    return Ret;
}


static inline m44f 
M44f_Translation(f32 X, f32 Y, f32 Z) {
    // NOTE(Momo): 
    // 1 0 0 x
    // 0 1 0 y
    // 0 0 1 z
    // 0 0 0 1
    //
    m44f Ret = M44f_Identity();
    Ret[0][3] = X;
    Ret[1][3] = Y;
    Ret[2][3] = Z;
    
    return Ret;
}

static inline m44f 
M44f_RotationX(f32 Rad) {
    // NOTE(Momo): 
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    
    f32 c = cos(Rad);
    f32 s = sin(Rad);
    m44f Ret = {};
    Ret[0][0] = 1.f;
    Ret[3][3] = 1.f;
    Ret[1][1] = c;
    Ret[1][2] = -s;
    Ret[2][1] = s;
    Ret[2][2] = c;
    
    return Ret;
}

static inline m44f 
M44f_RotationY(f32 rad) {
    // NOTE(Momo): 
    //  c  0  s  0
    //  0  1  0  0
    // -s  0  c  0
    //  0  0  0  1
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m44f Ret = {};
    Ret[0][0] = c;
    Ret[0][2] = s;
    Ret[1][1] = 1.f;
    Ret[2][0] = -s;
    Ret[2][2] = c;
    Ret[3][3] = 1.f;
    
    return Ret;
}

static inline m44f 
M44f_RotationZ(f32 rad) {
    // NOTE(Momo): 
    //  c -s  0  0
    //  s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m44f Ret = {};
    Ret[0][0] = c;
    Ret[0][1] = -s;
    Ret[1][0] = s;
    Ret[1][1] = c;
    Ret[2][2] = 1.f;
    Ret[3][3] = 1.f;
    
    return Ret;
}

static inline m44f
M44f_Scale(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    //  x  0  0  0
    //  0  y  0  0
    //  0  0  z  0
    //  0  0  0  1
    m44f Ret = {};
    Ret[0][0] = x;
    Ret[1][1] = y;
    Ret[2][2] = z;
    Ret[3][3] = 1.f;
    
    return Ret; 
}

static inline m44f 
M44f_Orthographic(f32 NdcLeft, f32 NdcRight,
                  f32 NdcBottom, f32 NdcTop,
                  f32 NdcNear, f32 NdcFar,
                  f32 Left, f32 Right, 
                  f32 Bottom, f32 Top,
                  f32 Near, f32 Far,
                  b8 FlipZ) 
{
    m44f Ret = {};
    Ret[0][0] = (NdcRight-NdcLeft)/(Right-Left);
    Ret[1][1] = (NdcTop-NdcBottom)/(Top-Bottom);
    Ret[2][2] = (FlipZ ? -1.f : 1.f) * (NdcFar-NdcNear)/(Far-Near);
    Ret[3][3] = 1.f;
    Ret[0][3] = -(Right+Left)/(Right-Left);
    Ret[1][3] = -(Top+Bottom)/(Top-Bottom);
    Ret[2][3] = -(Far+Near)/(Far-Near);
    
    return Ret;
}


//~ NOTE(Momo): Ray and lines
struct line2f {
    v2f Min;
    v2f Max;
};


struct ray2f {
    v2f Origin;
    v2f Dir;
};

static inline line2f
Line2f_CreateFromV2f(v2f Min, v2f Max) {
    line2f Ret = {};
    Ret.Min = Min;
    Ret.Max = Max;
    
    return Ret;
}

static inline line2f
Line2f_Create(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY) {
    v2f Min = { MinX, MinY };
    v2f Max = { MaxX, MaxY };
    
    return Line2f_CreateFromV2f(Min, Max);
    
}

static inline ray2f
Ray2f_CreateFromLine2f(line2f L) {
    ray2f Ret = {};
    Ret.Origin = L.Min;
    Ret.Dir = L.Max -L.Min;
    return Ret;
}

static inline void 
Ray2f_IntersectionTime(ray2f Lhs, ray2f Rhs, f32* LhsTimeResult, f32* RhsTimeResult) {
    f32 t1;
    f32 t2;
    
    v2f p1 = Lhs.Origin;
    v2f p2 = Rhs.Origin;
    v2f v1 = Lhs.Dir;
    v2f v2 = Rhs.Dir;
    
    
    t2 = (v1.x*p2.y - v1.x*p1.y - v1.y*p2.x + v1.y*p1.x)/(v1.y*v2.x - v1.x*v2.y);
    t1 = (p2.x + t2*v2.x - p1.x)/v1.x;
    
    *LhsTimeResult = t1;
    *RhsTimeResult = t2;
}

static inline v2f
Ray2f_Point(ray2f Ray, f32 Time) {
    // O + D * T
    v2f DirTime = Ray.Dir * Time;
    v2f Ret = Ray.Origin + DirTime;
    return Ret;
}

//~ NOTE(Momo): Quad
// TODO(Momo): I don't think this belongs here?
// A bit to 'freeform' to actually use
struct quad2f {
    v2f Points[4];
};

// TODO(Momo): This definitely doesn't belong here
static inline quad2f
Quad2f_CreateDefaultUV() {
    quad2f Ret = {};
    Ret.Points[0] = { 0.f, 1.f };
    Ret.Points[1] = { 1.f, 1.f };
    Ret.Points[2] = { 1.f, 0.f };
    Ret.Points[3] = { 0.f, 0.f };
    
    return Ret;
}


static inline quad2f
Aabb2f_To_Quad2f(aabb2f Aabb) {
    quad2f Ret = {};
    Ret.Points[0] = { Aabb.Min.x, Aabb.Max.y };
    Ret.Points[1] = { Aabb.Max.x, Aabb.Max.y };
    Ret.Points[2] = { Aabb.Max.x, Aabb.Min.y };
    Ret.Points[3] = { Aabb.Min.x, Aabb.Min.y };
    
    return Ret;
}

#endif