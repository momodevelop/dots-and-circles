#ifndef __MOMO_MATH__
#define __MOMO_MATH__



// NOTE(Momo): to future self: We wrap the math functions IN CASE we 
// want to have specialized versions for different systems (using SSE etc)
//
// I guess a good rule of thumb is that the GAME CODE should never directly
// use standard library functions IN CASE we cannot use them!
//

// NOTE(Momo): Constants
#define MM_Math_Pi32 3.14159265358979323846264338327950288f
#define MM_Math_Epsilon32 1.19209290E-07f
#define MM_Math_Tau32 MM_Math_Pi32 * 2.f

#define GenerateSubscriptOp(Amt) inline auto& operator[](u32 I) { Assert(I < Amt); return elements[I]; }



// NOTE(Momo): Common Functions
static inline b32
MM_Math_F32IsEqual(f32 lhs, f32 rhs) {
    f32 Test = AbsOf(lhs - rhs);
    return AbsOf(lhs - rhs) <= MM_Math_Epsilon32;
}

static inline f32 
MM_Math_DegToRad(f32 degrees) {
    return degrees * MM_Math_Pi32 / 180.f;
}

static inline f32 
MM_Math_RadToDeg(f32 radians) {
    return radians * 180.f / MM_Math_Pi32;
}

static inline f32 
MM_Math_Sin(f32 x) {
    return sinf(x);
}

static inline f32 
MM_Math_Cos(f32 x) {
    return cosf(x);
}

static inline f32 
MM_Math_Tan(f32 x) {
    return tanf(x);
}


static inline f32 
MM_Math_Sqrt(f32 x) {
    return sqrtf(x);
}

static inline f32 
MM_Math_ASin(f32 x) {
    return asinf(x);
}

static inline f32 
MM_Math_ACos(f32 x) {
    return acosf(x);
}

static inline f32 
MM_Math_ATan(f32 x) {
    return atanf(x);
}

static inline f32
MM_Math_Pow(f32 b, f32 e) {
    return powf(b,e);
}

//~ NOTE(Momo): Vectors
struct MM_V2f {
    union {
        f32 elements[2];
        struct {
            f32 x;
            f32 y;
        };
        struct {
            f32 U;
            f32 v;
        };
        struct {
            f32 W;
            f32 H;
        };
    };
    GenerateSubscriptOp(2)
};

struct MM_V2u {
    union {
        u32 elements[2];
        struct {
            u32 x;
            u32 y;
        };
        struct {
            u32 U;
            u32 v;
        };
        struct {
            u32 W;
            u32 H;
        };
    };
    GenerateSubscriptOp(2)
};

struct MM_V2i {
    union {
        s32 elements[2];
        struct {
            s32 x;
            s32 y;
        };
        struct {
            s32 U;
            s32 v;
        };
        struct {
            s32 W;
            s32 H;
        };
    };
    GenerateSubscriptOp(2)
};

struct MM_V3f {
    union {
        f32 elements[3];
        struct {
            union {
                MM_V2f XY;
                struct {
                    f32 x, y;
                };
            };
            f32 z;
        };
        
        struct {
            union {
                MM_V2f WH;
                struct {
                    f32 W, H;
                };
            };
            f32 D;
        }; 
    };
    GenerateSubscriptOp(3)
};

struct MM_V4f {
    union {
        f32 elements[4];
        struct {
            union {
                MM_V3f XYZ;
                struct {
                    f32 x, y, z;
                };
            }; 
            f32 W;
        };
    };
    GenerateSubscriptOp(4);
};

//~ NOTE(Momo): MM_V2f functions
static inline MM_V2f
MM_V2f_Create(f32 x, f32 y) {
    MM_V2f ret = {};
    ret.x = x;
    ret.y = y;
    
    return ret;
}

static inline MM_V2f
MM_V2f_CreateFromV2u(MM_V2u v) {
    return MM_V2f_Create((f32)v.x, (f32)v.y);
}

static inline MM_V2f 
MM_V2f_Add(MM_V2f lhs, MM_V2f rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

static inline MM_V2f
MM_V2f_Sub(MM_V2f lhs, MM_V2f rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}


static inline MM_V2f
MM_V2f_Mul(MM_V2f lhs, f32 rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}


static inline MM_V2f
MM_V2f_Div(MM_V2f lhs, f32 rhs) {
    Assert(!MM_Math_F32IsEqual(rhs, 0));
    lhs.x /= rhs;
    lhs.y /= rhs;
    
    return lhs;
}

static inline MM_V2f
MM_V2f_Ratio(MM_V2f lhs, MM_V2f rhs) {
    Assert(!MM_Math_F32IsEqual(rhs.x, 0));
    Assert(!MM_Math_F32IsEqual(rhs.y, 0));
    
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    
    return lhs;
}

static inline MM_V2f
MM_V2f_Neg(MM_V2f v){
    v.x = -v.x;
    v.y = -v.y;
    return v;
}

static inline b8 
MM_V2f_IsEqual(MM_V2f lhs, MM_V2f rhs) {
    return 
        MM_Math_F32IsEqual(lhs.x, rhs.x) && 
        MM_Math_F32IsEqual(lhs.y, rhs.y);
}

static inline f32 
MM_V2f_Dot(MM_V2f lhs, MM_V2f rhs) {
    f32 ret = {};
    ret = lhs.x * rhs.x + lhs.y * rhs.y;
    return ret;
}

static inline MM_V2f 
MM_V2f_Midpoint(MM_V2f lhs, MM_V2f rhs)  { 
    MM_V2f LR = MM_V2f_Add(lhs, rhs);
    MM_V2f ret = MM_V2f_Div(LR, 2.f);
    return ret; 
}

static inline f32
MM_V2f_LengthSq(MM_V2f v) { 
    // NOTE(Momo): Dot Product trick!
    return MM_V2f_Dot(v, v);
}

static inline f32
MM_V2f_DistanceSq(MM_V2f lhs, MM_V2f rhs) {
    MM_V2f v = MM_V2f_Sub(rhs, lhs);
    f32 ret = MM_V2f_LengthSq(v); 
    return ret;
}

static inline f32
MM_V2f_Distance(MM_V2f lhs, MM_V2f rhs)  { 
    return MM_Math_Sqrt(MM_V2f_DistanceSq(lhs, rhs)); 
}

static inline f32 
MM_V2f_Length(MM_V2f lhs)  { 
    return MM_Math_Sqrt(MM_V2f_LengthSq(lhs));
}

static inline MM_V2f 
MM_V2f_Normalize(MM_V2f v)  {
    f32 len = MM_V2f_Length(v);
    MM_V2f ret = MM_V2f_Div(v, len);
    return ret;
}

static inline f32
MM_V2f_AngleBetween(MM_V2f lhs, MM_V2f rhs) {
    f32 LLen = MM_V2f_Length(lhs);
    f32 RLen = MM_V2f_Length(rhs);
    f32 lhs_dot_rhs = MM_V2f_Dot(lhs,rhs);
    f32 ret = MM_Math_ACos(lhs_dot_rhs/(LLen * RLen));
    
    return ret;
}

static inline b32
MM_V2f_IsPerp(MM_V2f lhs, MM_V2f rhs) { 
    f32 lhs_dot_rhs = MM_V2f_Dot(lhs,rhs);
    return MM_Math_F32IsEqual(lhs_dot_rhs, 0); 
}


static inline b32 
MM_V2f_IsSameDir(MM_V2f lhs, MM_V2f rhs) { 
    f32 lhs_dot_rhs = MM_V2f_Dot(lhs,rhs);
    return lhs_dot_rhs > 0; 
}


static inline b32 
MM_V2f_IsOppDir(MM_V2f lhs, MM_V2f rhs) { 
    f32 lhs_dot_rhs = MM_V2f_Dot(lhs,rhs);
    return lhs_dot_rhs < 0;
}

static inline MM_V2f 
MM_V2f_Project(MM_V2f from, MM_V2f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_lensq = MM_V2f_LengthSq(to);
    Assert(!MM_Math_F32IsEqual(to_lensq, 0)); 
    
    f32 to_dot_from = MM_V2f_Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_lensq;
    MM_V2f ret = MM_V2f_Mul(to, unit_projection_scalar);
    return ret;
}

static inline MM_V2f
MM_V2f_Rotate(MM_V2f v, f32 rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 C = MM_Math_Cos(rad);
    f32 S = MM_Math_Sin(rad);
    
    MM_V2f ret = {};
    ret.x = (C * v.x) - (S * v.y);
    ret.y = (S * v.x) + (C * v.y);
    return ret;
}


//~ NOTE(Momo): MM_V2u Functions
static inline MM_V2u 
MM_V2u_Create(u32 x, u32 y) {
    MM_V2u ret = {};
    ret.x = x;
    ret.y = y;
    
    return ret;
}

//~ NOTE(Momo): MM_V3f Functions
static inline MM_V3f
MM_V3f_Create(f32 x, f32 y, f32 z) {
    MM_V3f ret = {};
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

static inline MM_V3f 
MM_V3f_Add(MM_V3f lhs, MM_V3f rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

static inline MM_V3f
MM_V3f_Sub(MM_V3f lhs, MM_V3f rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}


static inline MM_V3f
MM_V3f_Mul(MM_V3f lhs, f32 rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;
    return lhs;
}


static inline MM_V3f
MM_V3f_Div(MM_V3f lhs, f32 rhs) {
    Assert(!MM_Math_F32IsEqual(rhs, 0));
    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;
    return lhs;
}

static inline MM_V3f
MM_V3f_Neg(MM_V3f v){
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    return v;
}

static inline b8 
MM_V3f_IsEqual(MM_V3f lhs, MM_V3f rhs) {
    return 
        MM_Math_F32IsEqual(lhs.x, rhs.x) && 
        MM_Math_F32IsEqual(lhs.y, rhs.y) &&
        MM_Math_F32IsEqual(lhs.z, rhs.z);
}

static inline f32 
MM_V3f_Dot(MM_V3f lhs, MM_V3f rhs) {
    f32 ret = {};
    ret = lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    return ret;
}

static inline MM_V3f 
MM_V3f_Midpoint(MM_V3f lhs, MM_V3f rhs)  { 
    MM_V3f LR = MM_V3f_Add(lhs, rhs);
    MM_V3f ret = MM_V3f_Div(LR, 2.f);
    return ret; 
}

static inline f32
MM_V3f_LengthSq(MM_V3f v) { 
    // NOTE(Momo): Dot Product trick!
    return MM_V3f_Dot(v, v);
}

static inline f32
MM_V3f_DistanceSq(MM_V3f lhs, MM_V3f rhs) {
    MM_V3f v = MM_V3f_Sub(rhs, lhs);
    f32 ret = MM_V3f_LengthSq(v); 
    return ret;
}

static inline f32
MM_V3f_Distance(MM_V3f lhs, MM_V3f rhs)  { 
    return MM_Math_Sqrt(MM_V3f_DistanceSq(lhs, rhs)); 
}

static inline f32 
MM_V3f_Length(MM_V3f lhs)  { 
    return MM_Math_Sqrt(MM_V3f_LengthSq(lhs));
}

static inline MM_V3f 
MM_V3f_Normalize(MM_V3f v)  {
    f32 len = MM_V3f_Length(v);
    MM_V3f ret = MM_V3f_Div(v, len);
    return ret;
}

static inline f32
MM_V3f_AngleBetween(MM_V3f lhs, MM_V3f rhs) {
    f32 LLen = MM_V3f_Length(lhs);
    f32 RLen = MM_V3f_Length(rhs);
    f32 lhs_dot_rhs = MM_V3f_Dot(lhs,rhs);
    f32 ret = MM_Math_ACos(lhs_dot_rhs/(LLen * RLen));
    
    return ret;
}


static inline b32
MM_V3f_IsPerpendicular(MM_V3f lhs, MM_V3f rhs) { 
    f32 lhs_dot_rhs = MM_V3f_Dot(lhs,rhs);
    return MM_Math_F32IsEqual(lhs_dot_rhs, 0); 
}


static inline b32 
MM_V3f_IsSameDir(MM_V3f lhs, MM_V3f rhs) { 
    f32 lhs_dot_rhs = MM_V3f_Dot(lhs,rhs);
    return lhs_dot_rhs > 0; 
}


static inline b32 
MM_V3f_IsOppDir(MM_V3f lhs, MM_V3f rhs) { 
    f32 lhs_dot_rhs = MM_V3f_Dot(lhs,rhs);
    return lhs_dot_rhs < 0;
}

static inline MM_V3f 
MM_V3f_Project(MM_V3f from, MM_V3f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_lensq = MM_V3f_LengthSq(to);
    Assert(!MM_Math_F32IsEqual(to_lensq, 0)); 
    
    f32 to_dot_from = MM_V3f_Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_lensq;
    MM_V3f ret = MM_V3f_Mul(to, unit_projection_scalar);
    return ret;
}

static inline MM_V3f 
MM_V2f_ToV3f(MM_V2f v) {
    MM_V3f ret = {};
    ret.x = v.x;
    ret.y = v.y;
    ret.z = 0.f;
    
    return ret;
}

static inline MM_V2f
MM_V2u_ToV2f(MM_V2u v) {
    MM_V2f ret = {};
    ret.x = (f32)v.x;
    ret.y = (f32)v.y;
    
    return ret;
}

static inline MM_V2f 
MM_V2i_ToV2f(MM_V2i v) {
    MM_V2f ret = {};
    ret.x = (f32)v.x;
    ret.y = (f32)v.y;
    
    return ret;
}

//~ NOTE(Momo): AABB 
struct MM_Aabb2i {
    MM_V2i min;
    MM_V2i max;
};

struct MM_Aabb2f {
    MM_V2f min;
    MM_V2f max;
};

struct MM_Aabb2u {
    MM_V2u min;
    MM_V2u max;
};

struct MM_Aabb3f {
    MM_V3f min;
    MM_V3f max;
};


static inline MM_Aabb2u
MM_Aabb2u_CreateFromV2u(MM_V2u min, MM_V2u max) {
    MM_Aabb2u ret = {};
    ret.min = min;
    ret.max = max;
    
    return ret;
}

static inline MM_Aabb2u
MM_Aabb2u_Create(u32 min_x, u32 min_y, u32 max_x, u32 max_y) {
    MM_V2u min = MM_V2u_Create(min_x, min_y);
    MM_V2u max = MM_V2u_Create(max_x, max_y);
    
    return MM_Aabb2u_CreateFromV2u(min, max);
    
}


static inline u32
MM_Aabb2u_Width(MM_Aabb2u a) {
    return a.max.x - a.min.x;
}

static inline u32
MM_Aabb2u_Height(MM_Aabb2u a) {
    return a.max.y - a.min.y; 
}

static inline MM_V2u
MM_Aabb2u_Dimensions(MM_Aabb2u a) {
    MM_V2u ret = {};
    ret.W = MM_Aabb2u_Width(a);
    ret.H = MM_Aabb2u_Height(a);
    
    return ret;
}

static inline MM_Aabb2f
MM_Aabb2f_Mul(MM_Aabb2f lhs, f32 rhs) {
    lhs.min = MM_V2f_Mul(lhs.min, rhs);
    lhs.max = MM_V2f_Mul(lhs.max, rhs);
    return lhs;
}

static inline f32
MM_Aabb2f_Width(MM_Aabb2f a) {
    return a.max.x - a.min.x;
}

static inline f32
MM_Aabb2f_Height(MM_Aabb2f a) {
    return a.max.y - a.min.y; 
}


static inline MM_V2f
MM_Aabb2f_Dimensions(MM_Aabb2f a) {
    MM_V2f ret = {};
    ret.W = MM_Aabb2f_Width(a);
    ret.H = MM_Aabb2f_Height(a);
    
    return ret;
}


static inline MM_Aabb2u
MM_Aabb2u_CreateXYWH(u32 x, u32 y, u32 W, u32 H) {
    MM_Aabb2u ret = {};
    ret.min.x = x;
    ret.min.y = y;
    ret.max.x = x + W;
    ret.max.y = y + H;
    return ret;
}

static inline MM_Aabb2u
MM_Aabb2u_CreateWH(u32 W, u32 H) {
    MM_Aabb2u ret = {};
    ret.max.x = W;
    ret.max.y = H;
    
    return ret;
}


static inline MM_Aabb3f
MM_Aabb3f_CreateCentered(MM_V3f Dimensions, MM_V3f Anchor) {
    MM_Aabb3f ret = {};
    ret.min.x = Lerp(0.f, -Dimensions.W, Anchor.x);
    ret.max.x = Lerp(Dimensions.W, 0.f, Anchor.x);
    
    ret.min.y = Lerp(0.f, -Dimensions.H, Anchor.y);
    ret.max.y = Lerp(Dimensions.H, 0.f, Anchor.y);
    
    ret.min.z = Lerp(0.f, -Dimensions.D, Anchor.z);
    ret.max.z = Lerp(Dimensions.D, 0.f, Anchor.z);
    
    return ret; 
}

static inline f32
MM_Aabb2f_AspectRatio(MM_Aabb2f v) {
    return (f32)MM_Aabb2f_Width(v)/MM_Aabb2f_Height(v);
}

static inline MM_Aabb2f
MM_Aabb2i_ToAabb2f(MM_Aabb2i v) {
    MM_Aabb2f ret = {};
    ret.min = MM_V2i_ToV2f(v.min);
    ret.max = MM_V2i_ToV2f(v.max);
    return ret;
}

static inline MM_Aabb2f
MM_Aabb2u_ToAabb2f(MM_Aabb2u v) {
    MM_Aabb2f ret = {};
    ret.min = MM_V2u_ToV2f(v.min);
    ret.max = MM_V2u_ToV2f(v.max);
    return ret;
}

// NOTE(Momo): Gets the Normalized values of Aabb a based on another Aabb b
static inline MM_Aabb2f 
MM_Aabb2f_Ratio(MM_Aabb2f a, MM_Aabb2f b) {
    MM_Aabb2f ret = {};
    ret.min.x = Ratio(a.min.x, b.min.x, b.max.x);
    ret.min.y = Ratio(a.min.y, b.min.y, b.max.y);
    ret.max.x = Ratio(a.max.x, b.min.x, b.max.x);
    ret.max.y = Ratio(a.max.y, b.min.x, b.max.y);
    
    return ret;
}

static inline MM_Aabb2f 
MM_Aabb2u_Ratio(MM_Aabb2u a, MM_Aabb2u b) {
    MM_Aabb2f af = MM_Aabb2u_ToAabb2f(a);
    MM_Aabb2f bf = MM_Aabb2u_ToAabb2f(b);
    return MM_Aabb2f_Ratio(af, bf);
}

static inline MM_Aabb2u
MM_Aabb2u_Translate(MM_Aabb2u v, u32 x, u32 y) {
    MM_Aabb2u ret = v;
    ret.min.x += x;
    ret.min.y += y;
    ret.max.x += x;
    ret.max.y += y;
    
    return ret;
}

//~ NOTE(Momo): Circles
struct MM_Circle2f {
    MM_V2f origin;
    f32 radius;
};

static inline MM_Circle2f
MM_Circle2f_Create(MM_V2f origin, f32 radius) {
    MM_Circle2f ret = {};
    ret.origin = origin;
    ret.radius = radius;
    
    return ret;
}

static inline MM_Circle2f
MM_Circle2f_Offset(MM_Circle2f lhs, MM_V2f offset) {
    lhs.origin = MM_V2f_Add(lhs.origin, offset);
    return lhs;
}

//~ NOTE(Momo): 2x2 Matrices
union MM_M22f {
    MM_V2f elements[4];
    GenerateSubscriptOp(2);
};

static inline MM_M22f 
MM_M22f_Concat(MM_M22f lhs, MM_M22f rhs) {
    MM_M22f ret = {};
    for (u8 r = 0; r < 2; r++) { 
        for (u8 c = 0; c < 2; c++) { 
            for (u8 i = 0; i < 2; i++) 
                ret[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return ret;
}

static inline MM_V2f
MM_M22f_ConcatV2f(MM_M22f lhs, MM_V2f rhs) {
    MM_V2f ret = {};
    ret[0] = lhs[0][0] * rhs[0] + lhs[0][1] * rhs[1];
    ret[1] = lhs[1][0] * rhs[0] + lhs[1][1] * rhs[1];
    
    return ret;
}


static inline MM_M22f 
MM_M22f_Identity() {
    MM_M22f ret = {};
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    
    return ret;
}


static inline MM_M22f 
MM_M22f_Rotation(f32 rad) {
    // NOTE(Momo): 
    // c -s
    // s  c
    
    f32 c = MM_Math_Cos(rad);
    f32 s = MM_Math_Sin(rad);
    MM_M22f ret = {};
    ret[0][0] = c;
    ret[0][1] = -s;
    ret[1][0] = s;
    ret[1][1] = c;
    
    return ret;
}

//~ NOTE(Momo): 4x4 Matrices
union MM_M44f {
    MM_V4f elements[4];
    GenerateSubscriptOp(4);
};

static inline MM_M44f 
MM_M44f_Concat(MM_M44f lhs, MM_M44f rhs) {
    MM_M44f ret = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                ret[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return ret;
}

static inline MM_M44f 
MM_M44f_Identity() {
    MM_M44f ret = {};
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    ret[2][2] = 1.f;
    ret[3][3] = 1.f;
    
    return ret;
}

static inline MM_M44f 
MM_M44f_Transpose(MM_M44f m) {
    MM_M44f ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            ret[i][j] = m[j][i];
        }
    }
    
    return ret;
}


static inline MM_M44f 
MM_M44f_Translation(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    // 1 0 0 x
    // 0 1 0 y
    // 0 0 1 z
    // 0 0 0 1
    //
    MM_M44f ret = MM_M44f_Identity();
    ret[0][3] = x;
    ret[1][3] = y;
    ret[2][3] = z;
    
    return ret;
}

static inline MM_M44f 
MM_M44f_RotationX(f32 rad) {
    // NOTE(Momo): 
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    
    f32 c = MM_Math_Cos(rad);
    f32 s = MM_Math_Sin(rad);
    MM_M44f ret = {};
    ret[0][0] = 1.f;
    ret[3][3] = 1.f;
    ret[1][1] = c;
    ret[1][2] = -s;
    ret[2][1] = s;
    ret[2][2] = c;
    
    return ret;
}

static inline MM_M44f 
MM_M44f_RotationY(f32 rad) {
    // NOTE(Momo): 
    //  c  0  s  0
    //  0  1  0  0
    // -s  0  c  0
    //  0  0  0  1
    
    f32 c = MM_Math_Cos(rad);
    f32 s = MM_Math_Sin(rad);
    MM_M44f ret = {};
    ret[0][0] = c;
    ret[0][2] = s;
    ret[1][1] = 1.f;
    ret[2][0] = -s;
    ret[2][2] = c;
    ret[3][3] = 1.f;
    
    return ret;
}

static inline MM_M44f 
MM_M44f_RotationZ(f32 rad) {
    // NOTE(Momo): 
    //  c -s  0  0
    //  s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    
    f32 c = MM_Math_Cos(rad);
    f32 s = MM_Math_Sin(rad);
    MM_M44f ret = {};
    ret[0][0] = c;
    ret[0][1] = -s;
    ret[1][0] = s;
    ret[1][1] = c;
    ret[2][2] = 1.f;
    ret[3][3] = 1.f;
    
    return ret;
}

static inline MM_M44f
MM_M44f_Scale(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    //  x  0  0  0
    //  0  y  0  0
    //  0  0  z  0
    //  0  0  0  1
    MM_M44f ret = {};
    ret[0][0] = x;
    ret[1][1] = y;
    ret[2][2] = z;
    ret[3][3] = 1.f;
    
    return ret; 
}

static inline MM_M44f 
MM_M44f_Orthographic(f32 ndc_left, f32 ndc_right,
                     f32 ndc_bot, f32 ndc_top,
                     f32 ndc_near, f32 ndc_far,
                     f32 left, f32 right, 
                     f32 bottom, f32 top,
                     f32 near_clip, f32 far_clip,
                     b8 flip_z) 
{
    MM_M44f ret = {};
    ret[0][0] = (ndc_right-ndc_left)/(right-left);
    ret[1][1] = (ndc_top-ndc_bot)/(top-bottom);
    ret[2][2] = (flip_z ? -1.f : 1.f) * (ndc_far - ndc_near)/(far_clip - near_clip);
    ret[3][3] = 1.f;
    ret[0][3] = -(right+left)/(right-left);
    ret[1][3] = -(top+bottom)/(top-bottom);
    ret[2][3] = -(far_clip+near_clip)/(far_clip-near_clip);
    
    return ret;
}


//~ NOTE(Momo): ray and lines
struct MM_Line2f {
    MM_V2f min;
    MM_V2f max;
};


struct MM_Ray2f {
    MM_V2f origin;
    MM_V2f dir;
};

static inline MM_Line2f
Line2f_CreateFromV2f(MM_V2f min, MM_V2f max) {
    MM_Line2f ret = {};
    ret.min = min;
    ret.max = max;
    
    return ret;
}

static inline MM_Line2f
Line2f_Create(f32 min_x, f32 min_y, f32 max_x, f32 max_y) {
    MM_V2f min = MM_V2f_Create(min_x, min_y);
    MM_V2f max = MM_V2f_Create(max_x, max_y);
    
    return Line2f_CreateFromV2f(min, max);
    
}

static inline MM_Ray2f
Ray2f_CreateFromLine2f(MM_Line2f lhs) {
    MM_Ray2f ret = {};
    ret.origin = lhs.min;
    ret.dir = MM_V2f_Sub(lhs.max, lhs.min);
    return ret;
}

// TODO: Change this to just return multiple variables
static inline void 
Ray2f_IntersectionTime(MM_Ray2f lhs, 
                       MM_Ray2f rhs, 
                       f32* out_lhs_time, 
                       f32* out_rhs_time) {
    f32 t1;
    f32 t2;
    
    MM_V2f p1 = lhs.origin;
    MM_V2f p2 = rhs.origin;
    MM_V2f v1 = lhs.dir;
    MM_V2f v2 = rhs.dir;
    
    
    t2 = (v1.x*p2.y - v1.x*p1.y - v1.y*p2.x + v1.y*p1.x)/(v1.y*v2.x - v1.x*v2.y);
    t1 = (p2.x + t2*v2.x - p1.x)/v1.x;
    
    *out_lhs_time = t1;
    *out_rhs_time = t2;
}

static inline MM_V2f
Ray2f_Point(MM_Ray2f ray, f32 time) {
    // O + D * T
    MM_V2f DirTime = MM_V2f_Mul(ray.dir, time);
    MM_V2f ret = MM_V2f_Add(ray.origin, DirTime);
    return ret;
}

//~ NOTE(Momo): Quad
// TODO(Momo): I don't think this belongs here?
// a bit to 'freeform' to actually use
struct MM_Quad2f {
    MM_V2f points[4];
};

// TODO(Momo): This definitely doesn't belong here
static inline MM_Quad2f
MM_Quad2f_CreateDefaultUV() {
    MM_Quad2f ret = {};
    ret.points[0] = MM_V2f_Create(0.f, 1.f);
    ret.points[1] = MM_V2f_Create(1.f, 1.f);
    ret.points[2] = MM_V2f_Create(1.f, 0.f);
    ret.points[3] = MM_V2f_Create(0.f, 0.f);
    
    return ret;
}


static inline MM_Quad2f
MM_Aabb2f_ToQuad2f(MM_Aabb2f Aabb) {
    MM_Quad2f ret = {};
    ret.points[0] = MM_V2f_Create(Aabb.min.x, Aabb.max.y);
    ret.points[1] = MM_V2f_Create(Aabb.max.x, Aabb.max.y);
    ret.points[2] = MM_V2f_Create(Aabb.max.x, Aabb.min.y);
    ret.points[3] = MM_V2f_Create(Aabb.min.x, Aabb.min.y);
    
    return ret;
}

#endif 
