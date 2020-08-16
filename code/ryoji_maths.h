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

// NOTE(Momo): Constants
static constexpr f32 Pi32 = 3.14159265358979323846264338327950288f;
static constexpr f32 Epsilon32  = 1.19209290E-07f;
static constexpr f32 Tau32  = Pi32 * 2.f;

template<usize n, typename type = f32> 
struct vec {
    type E[n];
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

template<typename type>
struct vec<2, type> {
    union {
        type E[2];
        struct {
            type X;
            type Y;
        };
        struct {
            type U;
            type V;
        };
        struct {
            type W;
            type H;
        };
    };
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

template<typename type>
struct vec<3, type> {
    union {
        type E[3];
        struct {
            type X, Y, Z;
        };
        struct {
            type W, H, D;
        };
        struct {
            vec<2, type> XY;
            type _;
        };
        struct {
            vec<2, type> WH;
            type _;
        };
        
    };
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};


template<typename type>
struct vec<4, type> {
    union {
        type E[4];
        struct {
            type X, Y, Z, W;
        };
        struct {
            vec<3, type> XYZ;
            f32 _;
        };
    };
    
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index]; }
};
using v2f = vec<2, f32>;
using v3f = vec<3, f32>;
using v4f = vec<4, f32>;


///???
struct c3f {
    union {
        f32 E[3];
        struct {
            f32 R, G, B;
        };
        struct {
            f32 H, S, V;
        };
    };
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

struct c4f {
    union {
        f32 E[4];
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
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

struct m44f {
    f32 E[4][4];
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};


template<usize N, typename type = f32>
struct circle {
    vec<N, type> Origin;
    type Radius;
};
using circle2f = circle<2, f32>;
using circle3f = circle<3, f32>;

template<usize N, typename type = f32>
struct aabb {
    vec<N, type> Origin;
    vec<N, type> HalfDimensions;
};
using aabb2f = aabb<2, f32>;
using aabb3f = aabb<3, f32>;

template<usize N, typename type = f32>
struct rect {
    vec<N, type> Min;
    vec<N, type> Max;
};
using rect2f = rect<2, f32>;
using rect3f = rect<3, f32>;
using line2f = rect<2, f32>;
using line3f = rect<3, f32>;

template<usize N, typename type>
static inline type 
GetLength(rect<N, type> Rect, usize Dimension) {
    return Rect.Max[Dimension] - Rect.Min[Dimension]; 
}


template<usize N, typename type>
static inline type 
GetWidth(rect<N, type> Rect) {
    return GetLength(Rect, 0); 
}


template<usize N, typename type>
static inline type 
GetHeight(rect<N, type> Rect) {
    return GetLength(Rect, 1); 
}


template<usize N, typename type = f32>
struct quad {
    vec<N, type> Points[4];
};
using quad2f = quad<2, f32>;
using quad3f = quad<3, f32>;


// NOTE(Momo): Functions
template<typename type>
static inline b8 
IsEqual(type L, type R) {
    return L == R;
}

template<>
inline b8
IsEqual<f32>(f32 L, f32 R) {
    return Abs(L - R) <= Epsilon32;
}

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

template<usize N, typename type>
static inline vec<N, type> 
Add(vec<N, type> L, vec<N, type> R) {
    vec<N, type> Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret[i] = L[i] + R[i];
    }
    return Ret;
}

template<usize N, typename type>
static inline vec<N, type> 
Sub(vec<N, type> L, vec<N, type> R) {
    vec<N, type> Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret[i] = L[i] - R[i];
    }
    return Ret;
}

template<usize N, typename type>
static inline vec<N, type> 
Mul(vec<N, type> L, f32 R) {
    vec<N, type> Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret[i] = L[i] * R;
    }
    return Ret;
}

template<usize N, typename type>
static inline vec<N, type> 
Div(vec<N, type> L, f32 R) {
    Assert(!IsEqual(R, 0.f));
    vec<N, type> Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret[i] = L[i] / R;
    }
    return Ret;
}

template<usize N, typename type>
static inline vec<N, type> 
Negate(vec<N, type> V){
    vec<N, type> Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret[i] = -V[i]
    }
    return Ret;
}

template<usize N, typename type>
static inline b8 
IsEqual(vec<N, type> L, vec<N, type> R) {
    for (usize i = 0; i < N; ++i) {
        if (!IsEqual(L[i], R[i]))
            return false;
    }
}

template<usize N, typename type>
static inline f32 
Dot(vec<N, type> L, vec<N, type> R) {
    type Ret = {};
    for (usize i = 0; i < N; ++i) {
        Ret += L[i] * R[i];
    }
    return Ret;
}

template<usize N, typename type>
static inline vec<N, type> 
operator+(vec<N, type> L, vec<N, type> R)  { 
    return Add(L, R); 
}

template<usize N, typename type>
static inline vec<N, type> 
operator-(vec<N, type> L, vec<N, type> R)  { 
    return Sub(L, R);
}

template<usize N, typename type>
static inline vec<N, type> 
operator*(vec<N, type> L, f32 R)  { 
    return Mul(L, R);
}

template<usize N, typename type>
static inline vec<N, type> 
operator*(f32 L, vec<N, type> R)  { 
    return Mul(R, L);
}

template<usize N, typename type>
static inline f32 
operator*(vec<N, type> L, vec<N, type> R) {
    return Dot(L, R); 
}

template<usize N, typename type>
static inline vec<N, type> 
operator/(vec<N, type> L, f32 R)  { 
    return Div(L, R); 
}

template<usize N, typename type>
static inline vec<N, type>& 
operator+=(vec<N, type>& L, vec<N, type> R) {
    return L = L + R;
}

template<usize N, typename type>
static inline vec<N, type>& 
operator-=(vec<N, type>& L, vec<N, type> R) {
    return L = L - R;
}

template<usize N, typename type>
static inline vec<N, type>& 
operator*=(vec<N, type>& L, f32 R) {
    return L = L * R;
}

template<usize N, typename type>
static inline vec<N, type>& 
operator/=(vec<N, type>& L, f32 R) {
    return L = L / R;
}

template<usize N, typename type>
static inline bool 
operator==(vec<N, type> L, vec<N, type> R)  { 
    return IsEqual(L, R);
}

template<usize N, typename type>
static inline bool 
operator!=(vec<N, type> L, vec<N, type> R) { 
    return !(L == R); 
}

template<usize N, typename type>
static inline vec<N, type> 
operator-(vec<N, type> V)  { 
    return Negate(V); 
}


template<usize N, typename type>
static inline vec<N, type> 
Midpoint(vec<N, type> L, vec<N, type> R)  { 
    return (L + R) / 2; 
}

template<usize N, typename type>
static inline type
DistanceSq(vec<N, type> L, vec<N, type> R) { 
    type Ret = {};
    for(usize i = 0; i < N; ++i) {
        Ret += (R[i] - L[i]) * (R[i] - L[i]);
    }
    return Ret;
}


template<usize N, typename type>
static inline type
LengthSq(vec<N, type> V) { 
    return V * V;
}

template<usize N, typename type>
static inline type 
Distance(vec<N, type> L, vec<N, type> R)  { 
    return Sqrt(DistanceSq(L, R)); 
}

template<usize N, typename type>
static inline type 
Length(vec<N, type> L)  { 
    return Sqrt(LengthSq(L));
}

template<usize N, typename type>
static inline vec<N, type> 
Normalize(vec<N, type> L)  {
    vec<N, type> ret = L;
    type len = Length(L);
    ret /= len;
    return ret;
}

template<usize N, typename type>
static inline f32 
AngleBetween(vec<N, type> L, vec<N, type> R) {
    return ACos((L * R) / (Length(L) * Length(R)));
}

template<usize N, typename type>
static inline b8
IsPerpendicular(vec<N, type> L, vec<N, type> R) { 
    return IsEqual((L * R), 0); 
}

template<usize N, typename type>
static inline bool 
IsSameDir(vec<N, type> L, vec<N, type> R) { 
    return (L * R) > 0; 
}

template<usize N, typename type>
static inline bool 
IsOppDir(vec<N, type> L, vec<N, type> R) { 
    return (L * R) < 0;
}

template<usize N, typename type>
static inline vec<N, type> 
Project(vec<N, type> from, vec<N, type> to) { 
    return (to * from) / LengthSq(to) * to;
}


// Row major
static inline m44f 
operator*(m44f L, m44f R) {
    m44f res = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                res[r][c] += L[r][i] *  R[i][c]; 
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
TranslationMatrix(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f 
TranslationMatrix(v3f Vec) {
    return TranslationMatrix(Vec[0], Vec[1], Vec[2]);
}


static inline m44f 
RotationXMatrix(f32 rad) {
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
RotationYMatrix(f32 rad) {
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
RotationZMatrix(f32 rad) {
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
ScaleMatrix(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
}



static inline m44f
ScaleMatrix(v3f Vec) {
    return ScaleMatrix(Vec[0], Vec[1], Vec[2]);
}

// 
static inline m44f 
OrthographicMatrix(f32 NdcLeft, f32 NdcRight,
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
    Ret[2][3] = -(Far+Near)/(Far-Near);
    
    return Ret;
}


static inline m44f 
IdentityMatrix() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

static inline v2f
V2(v3f V) {
    return { V.X, V.Y };
}

static inline v3f 
V3(v2f V) {
    return { V.X, V.Y, 0.f };
}

static inline line2f
Line2(line3f Line) {
    return {
        V2(Line.Min),
        V2(Line.Max),
    };
}

static inline line3f
Line3(line2f Line) {
    return {
        V3(Line.Min),
        V3(Line.Max),
    };
}


static inline rect2f
Rect2(rect3f Rect) {
    return {
        V2(Rect.Min),
        V2(Rect.Max),
    };
}


static inline rect3f
Rect3(rect2f Rect) {
    return {
        V3(Rect.Min),
        V3(Rect.Max),
    };
}



struct ray2f {
    v2f Origin;
    v2f Direction;
};

static inline ray2f
Ray2(line2f Line) {
    ray2f Ret = {};
    Ret.Origin = Line.Min;
    Ret.Direction = Line.Max - Line.Min;
    return Ret;
}


struct get_intersection_time_result {
    f32 LhsTime, RhsTime;
};
static inline get_intersection_time_result 
GetIntersectionTime(ray2f Lhs, ray2f Rhs) {
    f32 t1;
    f32 t2;
    
    v2f p1 = Lhs.Origin;
    v2f p2 = Rhs.Origin;
    v2f v1 = Lhs.Direction;
    v2f v2 = Rhs.Direction;
    
    
    t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
    t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
    
    return { t1, t2 };
}

static inline v2f
GetPoint(ray2f Ray, f32 Time) {
    return Ray.Origin + Ray.Direction * Time;
}


#endif 