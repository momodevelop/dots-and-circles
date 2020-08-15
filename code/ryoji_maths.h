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

// NOTE(Momo): Structures
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
        struct {
            f32 W;
            f32 H;
        };
    };
    
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};


struct v3f {
    union {
        f32 E[3];
        struct {
            f32 X, Y, Z;
        };
        struct {
            f32 W, H, D;
        };
        struct {
            v2f XY;
            f32 _;
        };
        struct {
            v2f WH;
            f32 _;
        };
        
    };
    
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index];}
};

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
    };
    
    inline const auto& operator[](usize index) const { return E[index]; }
    inline auto& operator[](usize index) { return E[index]; }
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

struct circle2f {
    v2f Origin;
    f32 Radius;
};

struct circle3f {
    v3f Origin;
    f32 Radius;
};


struct aabb2f {
    v2f Origin;
    v2f HalfDimensions;
};

struct aabb3f {
    v3f Origin;
    v3f HalfDimensions;
};

struct rect2f {
    v2f Min;
    v2f Max;
};

struct rect3f {
    v3f Min;
    v3f Max;
};


struct line2f {
    v2f Min;
    v2f Max;
};

struct line3f {
    v3f Min;
    v3f Max;
};

struct quad2f {
    v2f Points[4];
};

struct quad3f {
    v3f Points[4];
};



// NOTE(Momo): Functions
static inline bool 
IsEqual(f32 L, f32 R) {
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

// v2f
static inline b8 
IsEqual(v2f L, v2f R) {
    return 
        IsEqual(L.X, R.X) && 
        IsEqual(L.Y, R.Y);
}

static inline v2f 
Add(v2f L, v2f R) {
    return { L.X+ R.X, L.Y + R.Y  };
}

static inline v2f 
Sub(v2f L, v2f R) {
    return { L.X- R.X, L.Y - R.Y };
}

static inline v2f 
Mul(v2f L, f32 R) {
    return { L.X* R, L.Y * R };
}

static inline v2f 
Div(v2f L, f32 R) {
    Assert(!IsEqual(R, 0.f));
    return { L.X/ R, L.Y / R };
}

static inline v2f 
Negate(v2f V){
    return {-V.X, -V.Y };
}

static inline f32 
Dot(v2f L, v2f  R) {
    return L.X * R.X + L.Y * R.Y;
}

static inline v2f 
operator+(v2f L, v2f R)  { 
    return Add(L, R); 
}

static inline v2f 
operator-(v2f L, v2f R)  { 
    return Sub(L, R);
}

static inline v2f 
operator*(v2f L, f32 R)  { 
    return Mul(L, R);
}

static inline v2f 
operator*(f32 L, v2f R)  { 
    return Mul(R, L);
}

static inline f32 
operator*(v2f L, v2f R) {
    return Dot(L, R); 
}

static inline v2f 
operator/(v2f L, f32 R)  { 
    return Div(L, R); 
}

static inline v2f& 
operator+=(v2f& L, v2f R) {
    return L = L + R;
}

static inline v2f& 
operator-=(v2f& L, v2f R) {
    return L = L - R;
}

static inline v2f& 
operator*=(v2f& L, f32 R) {
    return L = L * R;
}

static inline v2f& 
operator/=(v2f& L, f32 R) {
    return L = L / R;
}

static inline bool 
operator==(v2f L, v2f R)  { 
    return IsEqual(L, R);
}

static inline bool 
operator!=(v2f L, v2f R) { 
    return !(L == R); 
}

static inline v2f 
operator-(v2f L)  {  
    return { -L.X, -L.Y }; 
}


static inline f32 
LengthSq(v2f L) { 
    return L * L;
}

static inline f32 
Length(v2f L) { 
    return Sqrt(LengthSq(L));
}

static inline v2f 
Midpoint(v2f L, v2f R)  { 
    return (L + R) * 0.5f; 
}

static inline f32 
AngleBetween(v2f L, v2f R) {
    return ACos((L * R) / (Length(L) * Length(R)));
}

// v3f
static inline v3f 
Add(v3f L, v3f R) {
    return { L.X+ R.X, L.Y + R.Y, L.Z + R.Z };
}

static inline v3f 
Sub(v3f L, v3f R) {
    return { L.X- R.X, L.Y - R.Y, L.Z - R.Z };
}

static inline v3f 
Mul(v3f L, f32 R) {
    return { L.X* R, L.Y * R, L.Z * R };
}

static inline v3f 
Div(v3f L, f32 R) {
    Assert(!IsEqual(R, 0.f));
    return { L.X/ R, L.Y / R, L.Z / R };
}

static inline v3f 
Negate(v3f V){
    return {-V.X, -V.Y, -V.Z};
}

static inline b8 
IsEqual(v3f L, v3f R) {
    return 
        IsEqual(L.X, R.X) && 
        IsEqual(L.Y, R.Y) && 
        IsEqual(L.Z, R.Z);
}



static inline f32 
Dot(v3f L, v3f R) {
    return L.X * R.X + L.Y * R.Y + L.Z * R.Z;
}


static inline v3f 
operator+(v3f L, v3f R)  { 
    return Add(L, R); 
}

static inline v3f 
operator-(v3f L, v3f R)  { 
    return Sub(L, R);
}

static inline v3f 
operator*(v3f L, f32 R)  { 
    return Mul(L, R);
}

static inline v3f 
operator*(f32 L, v3f R)  { 
    return Mul(R, L);
}

static inline f32 
operator*(v3f L, v3f R) {
    return Dot(L, R); 
}

static inline v3f 
operator/(v3f L, f32 R)  { 
    return Div(L, R); 
}

static inline v3f& 
operator+=(v3f& L, v3f R) {
    return L = L + R;
}

static inline v3f& 
operator-=(v3f& L, v3f R) {
    return L = L - R;
}

static inline v3f& 
operator*=(v3f& L, f32 R) {
    return L = L * R;
}

static inline v3f& 
operator/=(v3f& L, f32 R) {
    return L = L / R;
}

static inline bool 
operator==(v3f L, v3f R)  { 
    return IsEqual(L, R);
}

static inline bool 
operator!=(v3f L, v3f R) { 
    return !(L == R); 
}

static inline v3f 
operator-(v3f L)  {  
    return { -L.X, -L.Y, -L.Z}; 
}

static inline v3f 
Midpoint(v3f L, v3f R)  { 
    return (L + R) * 0.5f; 
}

static inline f32 
DistanceSq(v3f L, v3f R) { 
    return 
        (R.X - L.X) * (R.X - L.X) + 
        (R.Y - L.Y) * (R.Y - L.Y) +
        (R.Z - L.Z) * (R.Z - L.Z);
}

static inline f32 
LengthSq(v3f L) { 
    return L * L;
}


static inline f32 
Distance(v3f L, v3f R)  { 
    return Sqrt(DistanceSq(L, R)); 
}

static inline f32 
Length(v3f L)  { 
    return Sqrt(LengthSq(L));
};

static inline v3f 
Normalize(v3f L)  {
    v3f ret = L;
    f32 len = Length(L);
    ret /= len;
    return ret;
}

static inline f32 
AngleBetween(v3f L, v3f R) {
    return ACos((L * R) / (Length(L) * Length(R)));
}

static inline bool 
IsPerpendicular(v3f L, v3f R) 
{ 
    return IsEqual((L * R), 0.f); 
}

static inline bool 
IsSameDir(v3f L, v3f R) { 
    return (L * R) > 0.f; 
}

static inline bool 
IsOppDir(v3f L, v3f R) { 
    return (L * R) < 0.f;
}

static inline v3f 
Project(v3f from, v3f to) { 
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