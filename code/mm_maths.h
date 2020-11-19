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
static constexpr f32 mmm_Pi32 = 3.14159265358979323846264338327950288f;
static constexpr f32 mmm_Epsilon32  = 1.19209290E-07f;
static constexpr f32 mmm_Tau32  = mmm_Pi32 * 2.f;

union mmm_v2f {
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

union mmm_v3f {
    f32 Elements[3];
    struct {
        union {
            mmm_v2f XY;
            struct {
                f32 X, Y;
            };
        };
        f32 Z;
    };
    
    struct {
        union {
            mmm_v2f WH;
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

union mmm_v4f {
    f32 Elements[4];
    struct
    {
        union
        {
            mmm_v3f XYZ;
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
            mmm_v3f RGB;
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

union mmm_v2u {
    u32 Elements[2];
    struct {
        u32 X, Y;
    };

    inline auto& operator[](usize I) {
        Assert(I < 2); 
        return Elements[I]; 
    }
};

union mmm_v2i {
    i32 Elements[2];
    struct {
        i32 X, Y;
    };

    inline auto& operator[](usize I) {
        Assert(I < 2); 
        return Elements[I]; 
    }
};

struct mmm_m44f {
    mmm_v4f Elements[4];

    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Elements[I];
    }
};



struct mmm_circle2f {
    mmm_v2f Origin;
    f32 Radius;
};

struct mmm_circle3f {
    mmm_v3f Origin;
    f32 Radius;
};

struct mmm_aabb2f {
    mmm_v2f Origin;
    mmm_v2f Radius;
};

struct mmm_aabb3f {
    mmm_v3f Origin;
    mmm_v3f Radius;
};

struct mmm_rect2i {
    mmm_v2i Min;
    mmm_v2i Max;
};

struct mmm_rect2u {
    mmm_v2u Min;
    mmm_v2u Max;
};

struct mmm_rect2f {
    mmm_v2f Min;
    mmm_v2f Max;
};

struct mmm_rect3f {
    mmm_v3f Min;
    mmm_v3f Max;
};

struct mmm_line2f {
    mmm_v2f Min;
    mmm_v2f Max;
};

struct mmm_line3f {
    mmm_v3f Min;
    mmm_v3f Max;
};

struct mmm_quad2f {
    mmm_v2f Points[4];
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }

};


struct mmm_quad3f {
    mmm_v3f Points[4];
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }


};


// mmm_rect2u
static inline u32
mmm_Width(mmm_rect2u Rect) {
    return Rect.Max.X - Rect.Min.X; 
}

static inline u32
mmm_Height(mmm_rect2u Rect) {
    return Rect.Max.Y - Rect.Min.Y; 
}

// mmm_rect2f
static inline f32
mmm_Width(mmm_rect2f Rect) {
    return Rect.Max.X - Rect.Min.X; 
}

static inline f32
mmm_Height(mmm_rect2f Rect) {
    return Rect.Max.Y - Rect.Min.Y; 
}

// NOTE(Momo): Common Functions
static inline b8
mmm_IsEqual(f32 L, f32 R) {
    return Abs(L - R) <= mmm_Epsilon32;
}

static inline f32 
mmm_DegToRad(f32 degrees) {
    return degrees * mmm_Pi32 / 180.f;
}

static inline f32 
mmm_RadToDeg(f32 radians) {
    return radians * 180.f / mmm_Pi32;
}

static inline f32 
mmm_Sin(f32 x) {
    return sinf(x);
}

static inline f32 
mmm_Cos(f32 x) {
    return cosf(x);
}

static inline f32 
mmm_Tan(f32 x) {
    return tanf(x);
}


static inline f32 
mmm_Sqrt(f32 x) {
    return sqrtf(x);
}

static inline f32 
mmm_ASin(f32 x) {
    return asinf(x);
}

static inline f32 
mmm_ACos(f32 x) {
    return acosf(x);
}

static inline f32 
mmm_ATan(f32 x) {
    return atanf(x);
}

static inline f32
mmm_Pow(f32 b, f32 e) {
    return powf(b,e);
}

// NOTE(Momo): mmm_v2f functions
static inline mmm_v2f
mmm_V2F(mmm_v3f V) {
    return { V.X, V.Y };
}


static inline mmm_v2f
mmm_V2F(mmm_v2u V) {
    return { (f32)V.X, (f32)V.Y };
}

static inline mmm_v2f
mmm_V2F(mmm_v2i V) {
    return { (f32)V.X, (f32)V.Y };
}


static inline mmm_v2f 
mmm_Add(mmm_v2f L, mmm_v2f R) {
    return { L.X + R.X, L.Y + R.Y };
}


static inline mmm_v2f 
mmm_Sub(mmm_v2f L, mmm_v2f R) {
    return { L.X - R.X, L.Y - R.Y };
}


static inline mmm_v2f 
mmm_Mul(mmm_v2f L, f32 R) {
    return { L.X * R, L.Y * R };
}


static inline mmm_v2f 
mmm_Div(mmm_v2f L, f32 R) {
    Assert(!mmm_IsEqual(R, 0.f));
    return { L.X / R, L.Y / R };
}


static inline mmm_v2f 
mmm_Negate(mmm_v2f V){
    return { -V.X, -V.Y };
}


static inline b8 
mmm_IsEqual(mmm_v2f L, mmm_v2f R) {
    return 
        mmm_IsEqual(L.X, R.X) &&
        mmm_IsEqual(L.Y, R.Y);
}


static inline f32 
mmm_Dot(mmm_v2f L, mmm_v2f R) {
    return L.X * R.X + L.Y * R.Y;
}


static inline mmm_v2f 
operator+(mmm_v2f L, mmm_v2f R)  { 
    return mmm_Add(L, R); 
}


static inline mmm_v2f 
operator-(mmm_v2f L, mmm_v2f R)  { 
    return mmm_Sub(L, R);
}


static inline mmm_v2f 
operator*(mmm_v2f L, f32 R)  { 
    return mmm_Mul(L, R);
}


static inline mmm_v2f 
operator*(f32 L, mmm_v2f R)  { 
    return mmm_Mul(R, L);
}


static inline f32 
operator*(mmm_v2f L, mmm_v2f R) {
    return mmm_Dot(L, R); 
}


static inline mmm_v2f 
operator/(mmm_v2f L, f32 R)  { 
    return mmm_Div(L, R); 
}


static inline mmm_v2f& 
operator+=(mmm_v2f& L, mmm_v2f R) {
    return L = L + R;
}


static inline mmm_v2f& 
operator-=(mmm_v2f& L, mmm_v2f R) {
    return L = L - R;
}


static inline mmm_v2f& 
operator*=(mmm_v2f& L, f32 R) {
    return L = L * R;
}


static inline mmm_v2f& 
operator/=(mmm_v2f& L, f32 R) {
    return L = L / R;
}


static inline bool 
operator==(mmm_v2f L, mmm_v2f R)  { 
    return mmm_IsEqual(L, R);
}


static inline bool 
operator!=(mmm_v2f L, mmm_v2f R) { 
    return !(L == R); 
}


static inline mmm_v2f 
operator-(mmm_v2f V)  { 
    return mmm_Negate(V); 
}

static inline mmm_v2f 
mmm_Midpoint(mmm_v2f L, mmm_v2f R)  { 
    return (L + R) / 2; 
}


static inline f32
mmm_DistanceSq(mmm_v2f L, mmm_v2f R) { 
    return 
        (R.X - L.X) * (R.X - L.X) + 
        (R.Y - L.Y) * (R.Y - L.Y);
}



static inline f32
mmm_LengthSq(mmm_v2f V) { 
    return V * V;
}


static inline f32 
mmm_Distance(mmm_v2f L, mmm_v2f R)  { 
    return mmm_Sqrt(mmm_DistanceSq(L, R)); 
}


static inline f32 
mmm_Length(mmm_v2f L)  { 
    return mmm_Sqrt(mmm_LengthSq(L));
}


static inline mmm_v2f 
mmm_Normalize(mmm_v2f L)  {
    mmm_v2f ret = L;
    f32 len = mmm_Length(L);
    ret /= len;
    return ret;
}


static inline f32 
mmm_AngleBetween(mmm_v2f L, mmm_v2f R) {
    return mmm_ACos((L * R) / (mmm_Length(L) * mmm_Length(R)));
}


static inline b8
mmm_IsPerpendicular(mmm_v2f L, mmm_v2f R) { 
    return mmm_IsEqual((L * R), 0); 
}


static inline bool 
mmm_IsSameDir(mmm_v2f L, mmm_v2f R) { 
    return (L * R) > 0; 
}


static inline bool 
mmm_IsOppDir(mmm_v2f L, mmm_v2f R) { 
    return (L * R) < 0;
}


static inline mmm_v2f 
mmm_Project(mmm_v2f from, mmm_v2f to) { 
    return (to * from) / mmm_LengthSq(to) * to;
}


// NOTE(Momo): mmm_v3f functions
static inline mmm_v3f 
mmm_V3F(mmm_v2f V) {
    return { V.X, V.Y, 0.f };
}

static inline mmm_v3f 
mmm_Add(mmm_v3f L, mmm_v3f R) {
    return { L.X + R.X, L.Y + R.Y, L.Z + R.Z };
}


static inline mmm_v3f 
mmm_Sub(mmm_v3f L, mmm_v3f R) {
    return { L.X - R.X, L.Y - R.Y, L.Z - R.Z };
}


static inline mmm_v3f 
mmm_Mul(mmm_v3f L, f32 R) {
    return { L.X * R, L.Y * R, L.Z * R };
}


static inline mmm_v3f 
mmm_Div(mmm_v3f L, f32 R) {
    Assert(!mmm_IsEqual(R, 0.f));
    return { L.X / R, L.Y / R, L.Z / R };
}


static inline mmm_v3f 
mmm_Negate(mmm_v3f V){
    return { -V.X, -V.Y, -V.Z };
}


static inline b8 
mmm_IsEqual(mmm_v3f L, mmm_v3f R) {
    return 
        mmm_IsEqual(L.X, R.X) &&
        mmm_IsEqual(L.Y, R.Y) &&
        mmm_IsEqual(L.Z, R.Z) ;
}


static inline f32 
mmm_Dot(mmm_v3f L, mmm_v3f R) {
    return L.X * R.X + L.Y * R.Y + L.Z * R.Z;
}


static inline mmm_v3f 
operator+(mmm_v3f L, mmm_v3f R)  { 
    return mmm_Add(L, R); 
}


static inline mmm_v3f 
operator-(mmm_v3f L, mmm_v3f R)  { 
    return mmm_Sub(L, R);
}


static inline mmm_v3f 
operator*(mmm_v3f L, f32 R)  { 
    return mmm_Mul(L, R);
}


static inline mmm_v3f 
operator*(f32 L, mmm_v3f R)  { 
    return mmm_Mul(R, L);
}


static inline f32 
operator*(mmm_v3f L, mmm_v3f R) {
    return mmm_Dot(L, R); 
}


static inline mmm_v3f 
operator/(mmm_v3f L, f32 R)  { 
    return mmm_Div(L, R); 
}


static inline mmm_v3f& 
operator+=(mmm_v3f& L, mmm_v3f R) {
    return L = L + R;
}


static inline mmm_v3f& 
operator-=(mmm_v3f& L, mmm_v3f R) {
    return L = L - R;
}


static inline mmm_v3f& 
operator*=(mmm_v3f& L, f32 R) {
    return L = L * R;
}


static inline mmm_v3f& 
operator/=(mmm_v3f& L, f32 R) {
    return L = L / R;
}


static inline bool 
operator==(mmm_v3f L, mmm_v3f R)  { 
    return mmm_IsEqual(L, R);
}


static inline bool 
operator!=(mmm_v3f L, mmm_v3f R) { 
    return !(L == R); 
}


static inline mmm_v3f 
operator-(mmm_v3f V)  { 
    return mmm_Negate(V); 
}


static inline mmm_rect2f
operator*(mmm_rect2f Rect, f32 F) {
    return {
        Rect.Min * F,
        Rect.Max * F,
    };
}


static inline mmm_v3f 
mmm_Midpoint(mmm_v3f L, mmm_v3f R)  { 
    return (L + R) / 2; 
}


static inline f32
mmm_DistanceSq(mmm_v3f L, mmm_v3f R) { 
    return 
        (R.X - L.X) * (R.X - L.X) + 
        (R.Y - L.Y) * (R.Y - L.Y) +
        (R.Z - L.Z) * (R.Z - L.Z);
}



static inline f32
mmm_LengthSq(mmm_v3f V) { 
    return V * V;
}


static inline f32
Distance(mmm_v3f L, mmm_v3f R)  { 
    return mmm_Sqrt(mmm_DistanceSq(L, R)); 
}


static inline f32 
mmm_Length(mmm_v3f L)  { 
    return mmm_Sqrt(mmm_LengthSq(L));
}


static inline mmm_v3f 
mmm_Normalize(mmm_v3f L)  {
    mmm_v3f ret = L;
    f32 len = mmm_Length(L);
    ret /= len;
    return ret;
}


static inline f32 
mmm_AngleBetween(mmm_v3f L, mmm_v3f R) {
    return mmm_ACos((L * R) / (mmm_Length(L) * mmm_Length(R)));
}


static inline b8
mmm_IsPerpendicular(mmm_v3f L, mmm_v3f R) { 
    return mmm_IsEqual((L * R), 0); 
}


static inline bool 
mmm_IsSameDir(mmm_v3f L, mmm_v3f R) { 
    return (L * R) > 0; 
}


static inline bool 
mmm_IsOppDir(mmm_v3f L, mmm_v3f R) { 
    return (L * R) < 0;
}


static inline mmm_v3f 
mmm_Project(mmm_v3f from, mmm_v3f to) { 
    return (to * from) / mmm_LengthSq(to) * to;
}

// Intersections
static inline b32
mmm_IsIntersecting(mmm_circle2f L, mmm_circle2f R) {
	f32 distsq = mmm_DistanceSq(L.Origin, R.Origin);
	f32 rsq = L.Radius + R.Radius;
    rsq *= rsq;
	return distsq < rsq;
}

static inline b32
mmm_IsIntersecting(mmm_circle3f L, mmm_circle3f R) {
	f32 distsq = mmm_DistanceSq(L.Origin, R.Origin);
	f32 rsq = L.Radius + R.Radius;
    rsq *= rsq;
	return distsq < rsq;
}

// Row major
static inline mmm_m44f 
operator*(mmm_m44f L, mmm_m44f R) {
    mmm_m44f res = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                res[r][c] += L[r][i] *  R[i][c]; 
        } 
    } 
    return res;
}


static inline mmm_m44f 
mmm_Transpose(mmm_m44f M) {
    mmm_m44f Ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            Ret[i][j] = M[j][i];
        }
    }
    
    return Ret;
}


static inline mmm_m44f 
mmm_Translation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline mmm_m44f 
mmm_Translation(mmm_v3f Vec) {
    return mmm_Translation(Vec.X, Vec.Y, Vec.Z);
}


static inline mmm_m44f 
mmm_RotationX(f32 rad) {
    f32 c = mmm_Cos(rad);
    f32 s = mmm_Sin(rad);
    return {
        1.f,  0.f,   0.f, 0.f,
        0.f,  c,    -s,   0.f,  
        0.f,  s,     c,   0.f,
        0.f,  0.f,   0.f,  1.f
    };
}

static inline mmm_m44f 
mmm_RotationY(f32 rad) {
    f32 c = mmm_Cos(rad);
    f32 s = mmm_Sin(rad);
    return {
        c,   0.f, s,    0.f,
        0.f, 1.f, 0.f,  0.f,
        -s,  0.f, c,    0.f,
        0.f, 0.f, 0.f,  1.f
    };
    
}


static inline mmm_m44f 
mmm_RotationZ(f32 rad) {
    f32 c = mmm_Cos(rad);
    f32 s = mmm_Sin(rad);
    return {
        c,  -s,   0.f, 0.f,
        s,   c,   0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline mmm_m44f
mmm_Scale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
}



static inline mmm_m44f
mmm_Scale(mmm_v3f Vec) {
    return mmm_Scale(Vec.X, Vec.Y, Vec.Z);
}


static inline mmm_m44f 
mmm_Orthographic(f32 NdcLeft, f32 NdcRight,
                   f32 NdcBottom, f32 NdcTop,
                   f32 NdcNear, f32 NdcFar,
                   f32 Left, f32 Right, 
                   f32 Bottom, f32 Top,
                   f32 Near, f32 Far,
                   bool FlipZ) 
{
    mmm_m44f Ret = {};
    Ret[0][0] = (NdcRight-NdcLeft)/(Right-Left);
    Ret[1][1] = (NdcTop-NdcBottom)/(Top-Bottom);
    Ret[2][2] = (FlipZ ? -1.f : 1.f) * (NdcFar-NdcNear)/(Far-Near);
    Ret[3][3] = 1.f;
    Ret[0][3] = -(Right+Left)/(Right-Left);
    Ret[1][3] = -(Top+Bottom)/(Top-Bottom);
    Ret[2][3] = -(Far+Near)/(Far-Near);
    
    return Ret;
}


static inline mmm_m44f 
mmm_IdentityMatrix() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

static inline mmm_line2f
mmm_Line2F(mmm_line3f Line) {
    return {
        mmm_V2F(Line.Min),
        mmm_V2F(Line.Max),
    };
}

static inline mmm_line3f
mmm_Line3F(mmm_line2f Line) {
    return {
        mmm_V3F(Line.Min),
        mmm_V3F(Line.Max),
    };
}


static inline mmm_rect2f
mmm_Rect2F(mmm_rect3f Rect) {
    return {
        mmm_V2F(Rect.Min),
        mmm_V2F(Rect.Max),
    };
}

static inline mmm_rect2f 
mmm_Rect2F(mmm_rect2u Rect) {
    return {
        mmm_V2F(Rect.Min),
        mmm_V2F(Rect.Max),
    };
}

static inline mmm_rect2f 
mmm_Rect2F(mmm_rect2i Rect) {
    return {
        mmm_V2F(Rect.Min),
        mmm_V2F(Rect.Max),
    };
}

static inline mmm_rect3f
mmm_Rect3F(mmm_rect2f Rect) {
    return {
        mmm_V3F(Rect.Min),
        mmm_V3F(Rect.Max),
    };
}

static inline f32 
mmm_AspectRatio(mmm_rect2f R) {
    return mmm_Width(R)/mmm_Height(R);
}

static inline f32 
mmm_AspectRatio(mmm_rect2u R) {
    return (f32)mmm_Width(R)/mmm_Height(R);
}


// NOTE(Momo): Gets the mmm_Normalized values of Rect A based on another Rect B
static inline mmm_rect2f 
mmm_RatioRect(mmm_rect2f A, mmm_rect2f B) {
    return  {
        Ratio(A.Min.X, B.Min.X, B.Max.X),
        Ratio(A.Min.Y, B.Min.Y, B.Max.Y),
        Ratio(A.Max.X, B.Min.X, B.Max.X),
        Ratio(A.Max.Y, B.Min.X, B.Max.Y),
    };
}

static inline mmm_rect2f 
mmm_RatioRect(mmm_rect2u A, mmm_rect2u B) {
    return mmm_RatioRect(mmm_Rect2F(A), mmm_Rect2F(B));
}


struct mmm_ray2f {
    mmm_v2f Origin;
    mmm_v2f Direction;
};

static inline mmm_ray2f
mmm_Ray2F(mmm_line2f Line) {
    mmm_ray2f Ret = {};
    Ret.Origin = Line.Min;
    Ret.Direction = Line.Max - Line.Min;
    return Ret;
}

static inline void 
mmm_IntersectionTime(mmm_ray2f Lhs, mmm_ray2f Rhs, f32* LhsTimeResult, f32* RhsTimeResult) {
    f32 t1;
    f32 t2;
    
    mmm_v2f p1 = Lhs.Origin;
    mmm_v2f p2 = Rhs.Origin;
    mmm_v2f v1 = Lhs.Direction;
    mmm_v2f v2 = Rhs.Direction;
    
    
    t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
    t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
    
    *LhsTimeResult = t1;
    *RhsTimeResult = t2;
}

static inline mmm_v2f
mmm_PointOnRay(mmm_ray2f Ray, f32 Time) {
    return Ray.Origin + Ray.Direction * Time;
}


static inline mmm_quad2f
mmm_Quad2F(mmm_rect2f Rect) {
    return {
        Rect.Min.X, Rect.Max.Y, // top left
        Rect.Max.X, Rect.Max.Y, // top right
        Rect.Max.X, Rect.Min.Y, // bottom right
        Rect.Min.X, Rect.Min.Y, // bottom left
    };
}

#endif 
