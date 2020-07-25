#ifndef __RYOJI_MATH__
#define __RYOJI_MATH__

#include "ryoji.h"
#include <math.h>

// NOTE(Momo): To future self: We wrap the math functions IN CASE we 
// want to have specialized versions for different systems (using SSE etc)
//
// I guess a good rule of thumb is that the GAME CODE should never directly
// use standard library functions IN CASE we cannot use them!
//

static constexpr f32 Pi32 = 3.14159265358979323846264338327950288f;
static constexpr f32 Epsilon32  = 1.19209290E-07f;
static constexpr f32 Tau32  = Pi32 * 2.f;

static inline bool 
IsEqual(f32 lhs, f32 rhs) {
    return Abs(lhs - rhs) <= Epsilon32;
}

// Degrees and Radians
static inline f32 
DegToRad(f32 degrees) {
    return degrees * Pi32 / 180.f;
}

static inline f32 
RadToDeg(f32 radians) {
    return radians * 180.f / Pi32;
}


static inline f32 
Sin(f32 x) {
    return sinf(x);
}

static inline f32 
Cos(f32 x) {
    return cosf(x);
}

static inline f32 
Tan(f32 x) {
    return tanf(x);
}


static inline f32 
Sqrt(f32 x) {
    return sqrtf(x);
}

static inline f32 
ASin(f32 x) {
    return asinf(x);
}

static inline f32 
ACos(f32 x) {
    return acosf(x);
}

static inline f32 
ATan(f32 x) {
    return atanf(x);
}

static inline f32
Pow(f32 b, f32 e) {
    return powf(b,e);
}

struct v2f {
    union {
        f32 E[2];
        struct {
            f32 X;
            f32 Y;
        };
        struct {
            f32 U;
            f32 V;
        };
    };
    
    inline f32 operator[](usize index) const {
        Assert(index < 2);
        return E[index];
    };
};

struct v3f {
    union {
        f32 E[3];
        struct {
            f32 X, Y, Z;
        };
        struct {
            v2f XY;
            f32 _;
        };
        struct {
            f32 R, G, B;
        };
        struct {
            f32 H, S, V;
        };
    };
    
    inline f32 operator[](usize index) const { 
        Assert(index < 3);
        return E[index]; 
    }
};
using c3f = v3f;

struct v4f {
    union {
        f32 E[4];
        struct {
            f32 X, Y, Z, W;
        };
        struct {
            v3f XYZ;
            f32 _;
        };
        struct {
            f32 R, G, B, A;
        };
        struct {
            c3f RGB;
            f32 _;
        };
        struct {
            f32 H, S, V, A;
        };
        struct {
            c3f HSV;
            f32 _;
        };
    };
    
    inline f32 operator[](usize index) const { 
        Assert(index < 4);
        return E[index]; 
    }
};
using c4f = v4f;

// TODO(Momo): Templates....?
static inline v3f 
Add(v3f lhs, v3f rhs) {
    return { lhs.X+ rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z };
    
}

static inline v3f 
Sub(v3f lhs, v3f rhs) {
    return { lhs.X- rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z };
}

static inline v3f 
Mul(v3f lhs, f32 rhs) {
    return { lhs.X* rhs, lhs.Y * rhs, lhs.Z * rhs };
}

static inline v3f 
Div(v3f lhs, f32 rhs) {
    Assert(IsEqual(rhs, 0.f));
    return { lhs.X/ rhs, lhs.Y / rhs, lhs.Z / rhs };
}

static inline v3f 
Negate(v3f lhs){
    return {-lhs.X, -lhs.Y, -lhs.Z};
    
}
static inline bool 
IsEqual(v3f lhs, v3f rhs) {
    return 
        IsEqual(lhs.X, rhs.X) && 
        IsEqual(lhs.Y, rhs.Y) && 
        IsEqual(lhs.Z, rhs.Z);
}

static inline f32 
Dot(v3f lhs, v3f rhs) {
    return lhs.X* rhs.X+ lhs.Y * rhs.Y + lhs.Z * rhs.Z;
}

static inline v3f 
operator+(v3f lhs, v3f rhs)  { 
    return Add(lhs, rhs); 
}

static inline v3f 
operator-(v3f lhs, v3f rhs)  { 
    return Sub(lhs, rhs);
}

static inline v3f 
operator*(v3f lhs, f32 rhs)  { 
    return Mul(lhs, rhs);
}

static inline v3f 
operator*(float lhs, v3f rhs)  { 
    return Mul(rhs, lhs);
}

static inline f32 
operator*(v3f lhs, v3f rhs) {
    return Dot(lhs, rhs); 
}

static inline v3f 
operator/(v3f lhs, f32 rhs)  { 
    return Div(lhs, rhs); 
}

static inline v3f& 
operator+=(v3f& lhs, v3f rhs) {
    return lhs = lhs + rhs;
}

static inline v3f& 
operator-=(v3f& lhs, v3f rhs) {
    return lhs = lhs - rhs;
}

static inline v3f& 
operator*=(v3f& lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v3f& 
operator/=(v3f& lhs, f32 rhs) {
    return lhs = lhs / rhs;
}

static inline bool 
operator==(v3f lhs, v3f rhs)  { 
    return IsEqual(lhs, rhs);
}

static inline bool 
operator!=(v3f lhs, v3f rhs) { 
    return !(lhs == rhs); 
}

static inline v3f 
operator-(v3f lhs)  {  
    return { -lhs.X, -lhs.Y, -lhs.Z}; 
}

static inline v3f 
Midpoint(v3f lhs, v3f rhs)  { 
    return (lhs + rhs) * 0.5f; 
}

static inline f32 
DistSq(v3f lhs, v3f rhs) { 
    return (rhs.X- lhs.X) * (rhs.Y - lhs.Y) * (rhs.Z - lhs.Z);
}

static inline f32 
LenSq(v3f lhs) { 
    return lhs * lhs;	
}

static inline f32 
Dist(v3f lhs, v3f rhs)  { 
    return Sqrt(DistSq(lhs, rhs)); 
}

static inline f32 
Len(v3f lhs)  { 
    return Sqrt(LenSq(lhs));
};

static inline v3f 
Normalize(v3f lhs)  {
    v3f ret = lhs;
    f32 len = Len(lhs);
    if (IsEqual(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

static inline f32 
AngleBetween(v3f lhs, v3f rhs) {
    return ACos((lhs * rhs) / (Len(lhs) * Len(rhs)));
}

static inline bool 
IsPerpendicular(v3f lhs, v3f rhs) 
{ 
    return IsEqual((lhs * rhs), 0.f); 
}

static inline bool 
IsSameDir(v3f lhs, v3f rhs) { 
    return (lhs * rhs) > 0.f; 
}

static inline bool 
IsOppDir(v3f lhs, v3f rhs) { 
    return (lhs * rhs) < 0.f;
}

static inline v3f 
Project(v3f from, v3f to) { 
    return (to * from) / LenSq(to) * to;
}

// NOTE(Momo): I chose row major because it's easier to work on and OpenGL 
struct m44f {
    f32 E[4][4];
    
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

static inline m44f 
operator*(m44f lhs, m44f rhs) {
    m44f res = {};
    
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                res[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return res;
}


static inline m44f 
Transpose(m44f M) {
    m44f Ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            Ret[i][j] = M[j][i];
        }
    }
    
    return Ret;
}


static inline m44f 
MakeTranslationMatrix(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f 
MakeTranslationMatrix(v3f Vec) {
    return MakeTranslationMatrix(Vec[0], Vec[1], Vec[2]);
}


static inline m44f 
MakeRotationXMatrix(f32 rad) {
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    return {
        1.f,  0.f,   0.f, 0.f,
        0.f,  c,    -s,   0.f,  
        0.f,  s,     c,   0.f,
        0.f,  0.f,   0.f,  1.f
    };
}

static inline m44f 
MakeRotationYMatrix(f32 rad) {
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    return {
        c,   0.f, s,    0.f,
        0.f, 1.f, 0.f,  0.f,
        -s,  0.f, c,    0.f,
        0.f, 0.f, 0.f,  1.f
    };
    
}


static inline m44f 
MakeRotationZMatrix(f32 rad) {
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    return {
        c,  -s,   0.f, 0.f,
        s,   c,   0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f
MakeScaleMatrix(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
}



static inline m44f
MakeScaleMatrix(v3f Vec) {
    return MakeScaleMatrix(Vec[0], Vec[1], Vec[2]);
}

// 
static inline m44f 
MakeOrthographicMatrix(f32 NdcLeft, f32 NdcRight,
                       f32 NdcBottom, f32 NdcTop,
                       f32 NdcNear, f32 NdcFar,
                       f32 Left, f32 Right, 
                       f32 Bottom, f32 Top,
                       f32 Near, f32 Far,
                       bool FlipZ) 
{
    m44f Ret = {};
    Ret[0][0] = (NdcRight-NdcLeft)/(Right-Left);
    Ret[1][1] = (NdcTop-NdcBottom)/(Top-Bottom);
    Ret[2][2] = (FlipZ ? -1.f : 1.f) * (NdcFar-NdcNear)/(Far-Near);
    Ret[3][3] = 1.f;
    Ret[0][3] = -(Right+Left)/(Right-Left);
    Ret[1][3] = -(Top+Bottom)/(Top-Bottom);
    Ret[2][3] =  -(Far+Near)/(Far-Near);
    Ret[3][3] = 1.f;
    
    return Ret;
}


static inline m44f 
MakeIdentityMatrix() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}




#endif 