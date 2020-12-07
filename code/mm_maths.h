#ifndef __MOMO_MATH__
#define __MOMO_MATH__

#include "mm_core.h"
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

union v2f {
    f32 Elements[2];
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

    inline auto& operator[](usize I) {
        Assert(I < 2); 
        return Elements[I]; 
    }
};

union v3f {
    f32 Elements[3];
    struct {
        union {
            v2f XY;
            struct {
                f32 X, Y;
            };
        };
        f32 Z;
    };
    
    struct {
        union {
            v2f WH;
            struct {
                f32 W, H;
            };
        };
        f32 D;
    };
    
    struct {
        f32 R, G, B;
    };

    inline auto& operator[](usize I) {
        Assert(I < 3); 
        return Elements[I]; 
    }
};

union v4f {
    f32 Elements[4];
    struct
    {
        union
        {
            v3f XYZ;
            struct
            {
                f32 X, Y, Z;
            };
        };
        
        f32 W;
    };
    struct
    {
        union
        {
            v3f RGB;
            struct
            {
                f32 R, G, B;
            };
        };
        
        f32 A;
    };


    inline auto& operator[](usize I) {
        Assert(I < 4); 
        return Elements[I]; 
    }
};

union v2u {
    u32 Elements[2];
    struct {
        u32 X, Y;
    };

    inline auto& operator[](usize I) {
        Assert(I < 2); 
        return Elements[I]; 
    }
};

union v2i {
    i32 Elements[2];
    struct {
        i32 X, Y;
    };

    inline auto& operator[](usize I) {
        Assert(I < 2); 
        return Elements[I]; 
    }
};

struct m44f {
    v4f Elements[4];

    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Elements[I];
    }
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
    v2f Radius;
};

struct aabb3f {
    v3f Origin;
    v3f Radius;
};

struct rect2i {
    v2i Min;
    v2i Max;
};

struct rect2u {
    v2u Min;
    v2u Max;
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
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }

};


struct quad3f {
    v3f Points[4];
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }


};


// rect2u
static inline u32
Width(rect2u Rect) {
    return Rect.Max.X - Rect.Min.X; 
}

static inline u32
Height(rect2u Rect) {
    return Rect.Max.Y - Rect.Min.Y; 
}

// rect2f
static inline f32
Width(rect2f Rect) {
    return Rect.Max.X - Rect.Min.X; 
}

static inline f32
Height(rect2f Rect) {
    return Rect.Max.Y - Rect.Min.Y; 
}

// NOTE(Momo): Common Functions
static inline b8
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

// NOTE(Momo): v2f functions

static inline v2f 
V2f(v3f V) {
    return { V.X, V.Y };
}

static inline v2f
V2f(v2u V) {
    return { (f32)V.X, (f32)V.Y };
}

static inline v2f
V2f(v2i V) {
    return { (f32)V.X, (f32)V.Y };
}


static inline v2f 
Add(v2f L, v2f R) {
    return { L.X + R.X, L.Y + R.Y };
}


static inline v2f 
Sub(v2f L, v2f R) {
    return { L.X - R.X, L.Y - R.Y };
}


static inline v2f 
Mul(v2f L, f32 R) {
    return { L.X * R, L.Y * R };
}


static inline v2f 
Div(v2f L, f32 R) {
    Assert(!IsEqual(R, 0.f));
    return { L.X / R, L.Y / R };
}


static inline v2f 
Negate(v2f V){
    return { -V.X, -V.Y };
}


static inline b8 
IsEqual(v2f L, v2f R) {
    return 
        IsEqual(L.X, R.X) &&
        IsEqual(L.Y, R.Y);
}


static inline f32 
Dot(v2f L, v2f R) {
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
operator-(v2f V)  { 
    return Negate(V); 
}

static inline v2f 
Midpoint(v2f L, v2f R)  { 
    return (L + R) / 2; 
}


static inline f32
DistanceSq(v2f L, v2f R) { 
    return 
        (R.X - L.X) * (R.X - L.X) + 
        (R.Y - L.Y) * (R.Y - L.Y);
}



static inline f32
LengthSq(v2f V) { 
    return V * V;
}


static inline f32 
Distance(v2f L, v2f R)  { 
    return Sqrt(DistanceSq(L, R)); 
}


static inline f32 
Length(v2f L)  { 
    return Sqrt(LengthSq(L));
}


static inline v2f 
Normalize(v2f L)  {
    v2f ret = L;
    f32 len = Length(L);
    ret /= len;
    return ret;
}


static inline f32 
AngleBetween(v2f L, v2f R) {
    return ACos((L * R) / (Length(L) * Length(R)));
}


static inline b8
IsPerpendicular(v2f L, v2f R) { 
    return IsEqual((L * R), 0); 
}


static inline bool 
IsSameDir(v2f L, v2f R) { 
    return (L * R) > 0; 
}


static inline bool 
IsOppDir(v2f L, v2f R) { 
    return (L * R) < 0;
}


static inline v2f 
Project(v2f from, v2f to) { 
    return (to * from) / LengthSq(to) * to;
}


// NOTE(Momo): v3f functions
static inline v3f 
V3f(v2f V) {
    return { V.X, V.Y, 0.f };
}

static inline v3f 
Add(v3f L, v3f R) {
    return { L.X + R.X, L.Y + R.Y, L.Z + R.Z };
}


static inline v3f 
Sub(v3f L, v3f R) {
    return { L.X - R.X, L.Y - R.Y, L.Z - R.Z };
}


static inline v3f 
Mul(v3f L, f32 R) {
    return { L.X * R, L.Y * R, L.Z * R };
}


static inline v3f 
Div(v3f L, f32 R) {
    Assert(!IsEqual(R, 0.f));
    return { L.X / R, L.Y / R, L.Z / R };
}


static inline v3f 
Negate(v3f V){
    return { -V.X, -V.Y, -V.Z };
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
operator-(v3f V)  { 
    return Negate(V); 
}


static inline rect2f
operator*(rect2f Rect, f32 F) {
    return {
        Rect.Min * F,
        Rect.Max * F,
    };
}


static inline v3f 
Midpoint(v3f L, v3f R)  { 
    return (L + R) / 2; 
}


static inline f32
DistanceSq(v3f L, v3f R) { 
    return 
        (R.X - L.X) * (R.X - L.X) + 
        (R.Y - L.Y) * (R.Y - L.Y) +
        (R.Z - L.Z) * (R.Z - L.Z);
}



static inline f32
LengthSq(v3f V) { 
    return V * V;
}


static inline f32
Distance(v3f L, v3f R)  { 
    return Sqrt(DistanceSq(L, R)); 
}


static inline f32 
Length(v3f L)  { 
    return Sqrt(LengthSq(L));
}


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


static inline b8
IsPerpendicular(v3f L, v3f R) { 
    return IsEqual((L * R), 0); 
}


static inline bool 
IsSameDir(v3f L, v3f R) { 
    return (L * R) > 0; 
}


static inline bool 
IsOppDir(v3f L, v3f R) { 
    return (L * R) < 0;
}


static inline v3f 
Project(v3f from, v3f to) { 
    return (to * from) / LengthSq(to) * to;
}

// Intersections
static inline b32
IsIntersecting(circle2f L, circle2f R) {
	f32 distsq = DistanceSq(L.Origin, R.Origin);
	f32 rsq = L.Radius + R.Radius;
    rsq *= rsq;
	return distsq < rsq;
}

static inline b32
IsIntersecting(circle3f L, circle3f R) {
	f32 distsq = DistanceSq(L.Origin, R.Origin);
	f32 rsq = L.Radius + R.Radius;
    rsq *= rsq;
	return distsq < rsq;
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
M44fTranslation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f 
M44fTranslation(v2f Vec) {
    return M44fTranslation(Vec.X, Vec.Y, 0);
}

static inline m44f 
M44fTranslation(v3f Vec) {
    return M44fTranslation(Vec.X, Vec.Y, Vec.Z);
}


static inline m44f 
M44fRotationX(f32 rad) {
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
M44fRotationY(f32 rad) {
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
M44fRotationZ(f32 rad) {
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
M44fScale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f
M44fScale(v2f Vec) {
    return M44fScale(Vec.X, Vec.Y, 0.f);
}

static inline m44f
M44fScale(v3f Vec) {
    return M44fScale(Vec.X, Vec.Y, Vec.Z);
}


static inline m44f 
M44fOrthographic(f32 NdcLeft, f32 NdcRight,
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
M44fIdentity() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

static inline line2f
Line2f(line3f Line) {
    return {
        Line.Min.XY,
        Line.Max.XY,
    };
}

static inline line3f
Line3f(line2f Line) {
    return {
        V3f(Line.Min),
        V3f(Line.Max),
    };
}


static inline rect2f
Rect2f(rect3f Rect) {
    return {
        Rect.Min.XY,
        Rect.Max.XY,
    };
}

static inline rect2f 
Rect2f(rect2u Rect) {
    return {
        V2f(Rect.Min),
        V2f(Rect.Max),
    };
}

static inline rect2f 
Rect2f(rect2i Rect) {
    return {
        V2f(Rect.Min),
        V2f(Rect.Max),
    };
}

static inline rect3f
Rect3f(rect2f Rect) {
    return {
        V3f(Rect.Min),
        V3f(Rect.Max),
    };
}

static inline rect3f
CenteredRect(v3f Dimensions, v3f Anchor) {
    rect3f Ret = {};
    Ret.Min.X = Lerp(0, -Dimensions.W, Anchor.X);
    Ret.Max.X = Lerp(Dimensions.W, 0, Anchor.X);

    Ret.Min.Y = Lerp(0, -Dimensions.H, Anchor.Y);
    Ret.Max.Y = Lerp(Dimensions.H, 0, Anchor.Y);

    Ret.Min.Z = Lerp(0, -Dimensions.D, Anchor.Z);
    Ret.Max.Z = Lerp(Dimensions.D, 0, Anchor.Z);

    return Ret; 
}

static inline f32 
AspectRatio(rect2f R) {
    return Width(R)/Height(R);
}

static inline f32 
AspectRatio(rect2u R) {
    return (f32)Width(R)/Height(R);
}


// NOTE(Momo): Gets the Normalized values of Rect A based on another Rect B
static inline rect2f 
RatioRect(rect2f A, rect2f B) {
    return  {
        Ratio(A.Min.X, B.Min.X, B.Max.X),
        Ratio(A.Min.Y, B.Min.Y, B.Max.Y),
        Ratio(A.Max.X, B.Min.X, B.Max.X),
        Ratio(A.Max.Y, B.Min.X, B.Max.Y),
    };
}

static inline rect2f 
RatioRect(rect2u A, rect2u B) {
    return RatioRect(Rect2f(A), Rect2f(B));
}


struct ray2f {
    v2f Origin;
    v2f Direction;
};

static inline ray2f
LineToRay(line2f Line) {
    ray2f Ret = {};
    Ret.Origin = Line.Min;
    Ret.Direction = Line.Max - Line.Min;
    return Ret;
}

static inline void 
IntersectionTime(ray2f Lhs, ray2f Rhs, f32* LhsTimeResult, f32* RhsTimeResult) {
    f32 t1;
    f32 t2;
    
    v2f p1 = Lhs.Origin;
    v2f p2 = Rhs.Origin;
    v2f v1 = Lhs.Direction;
    v2f v2 = Rhs.Direction;
    
    
    t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
    t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
    
    *LhsTimeResult = t1;
    *RhsTimeResult = t2;
}

static inline v2f
PointOnRay(ray2f Ray, f32 Time) {
    return Ray.Origin + Ray.Direction * Time;
}


static inline quad2f
Quad2F(rect2f Rect) {
    return {
        Rect.Min.X, Rect.Max.Y, // top left
        Rect.Max.X, Rect.Max.Y, // top right
        Rect.Max.X, Rect.Min.Y, // bottom right
        Rect.Min.X, Rect.Min.Y, // bottom left
    };
}

#endif 
