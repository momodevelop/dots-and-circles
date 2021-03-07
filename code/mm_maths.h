#ifndef __MOMO_MATH__
#define __MOMO_MATH__



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

#define GenerateSubscriptOp(Amt) inline auto& operator[](usize I) { Assert(I < Amt); return Elements[I]; }

template<typename t, usize N>
struct vec {
    t Elements[N];
    GenerateSubscriptOp(N)
};

template<typename t>
struct vec<t, 2> {
    union {
        t Elements[2];
        struct {
            t X;
            t Y;
        };
        struct {
            t U;
            t V;
        };
        struct {
            t W;
            t H;
        };
    };
    GenerateSubscriptOp(2)
};
typedef vec<f32,2> v2f;
typedef vec<u32,2> v2u;
typedef vec<i32,2> v2i;

template<typename t>
struct vec<t ,3> {
    union {
        t Elements[3];
        struct {
            union {
                vec<t,2> XY;
                struct {
                    t X, Y;
                };
            };
            t Z;
        };
        
        struct {
            union {
                vec<t,2> WH;
                struct {
                    t W, H;
                };
            };
            t D;
        }; 
    };
    GenerateSubscriptOp(3)
};
typedef vec<f32,3> v3f;
typedef vec<u32,3> v3u;

template<typename t>
struct vec<t,4> {
    union {
        t Elements[4];
        struct {
            union {
                vec<t,3> XYZ;
                struct {
                    t X, Y, Z;
                };
            }; 
            f32 W;
        };
    };
    GenerateSubscriptOp(4)
};
typedef vec<f32,4> v4f;

struct m44f {
    v4f Elements[4];
    
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Elements[I];
    }
};

template<typename t, usize N>
struct circle {
    vec<t,N> Origin;
    t Radius;
};
using circle2f = circle<f32, 2>;

template<typename t, usize N>
struct aabb {
    vec<t,N> Min;
    vec<t,N> Max;
};
typedef aabb<u32,2> aabb2u;
typedef aabb<f32,2> aabb2f;
typedef aabb<i32,2> aabb2i;
typedef aabb<u32,3> aabb3u;
typedef aabb<f32,3> aabb3f;

template<typename t, usize N>
static inline aabb<t,N>
operator*(aabb<t,N> Lhs, t Rhs) {
    return { Lhs.Min * Rhs, Lhs.Max * Rhs };
}

template<typename t, usize N>
static inline aabb<t,N>
operator*(t Lhs, aabb<t,N> Rhs) {
    return { Rhs.Min * Lhs, Rhs.Max * Lhs };
}

template<typename t, usize N>
struct line {
    vec<t,N> Min;
    vec<t,N> Max;
};
typedef line<f32,2> line2f;
typedef line<f32,3> line3f;

template<typename t, usize N> 
struct quad {
    vec<t,N> Points[4];
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }
    
};
typedef quad<f32,2> quad2f;


template<typename t, usize N> 
static inline t
GetWidth(aabb<t,N> Aabb) {
    return Aabb.Max.X - Aabb.Min.X;
}

template<typename t, usize N>
static inline t
GetHeight(aabb<t,N> Aabb) {
    return Aabb.Max.Y - Aabb.Min.Y; 
}

// aabb2u
static inline aabb2u
Aabb2u(u32 X, u32 Y, u32 Width, u32 Height) {
    aabb2u Ret = {};
    Ret.Min.X = X;
    Ret.Min.Y = Y;
    Ret.Max.X = X + Width;
    Ret.Max.Y = Y + Height;
    return Ret;
}

// NOTE(Momo): Common Functions
static inline b32
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


template<typename t, usize n>
static inline vec<t,n> 
Add(vec<t,n> L, vec<t,n> R) {
    vec<t,n> Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret[I] = L[I] + R[I]; 
    }
    return Ret;
}

template<typename t, usize n>
static inline vec<t,n> 
Sub(vec<t,n> L, vec<t,n> R) {
    vec<t,n> Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret[I] = L[I] - R[I]; 
    }
    return Ret;
}


template<typename t, usize n>
static inline vec<t,n> 
Mul(vec<t,n> L, t R) {
    vec<t,n> Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret[I] = L[I] * R; 
    }
    return Ret;
}


template<typename t, usize n>
static inline vec<t,n>
Div(vec<t,n> L, f32 R) {
    Assert(!IsEqual(R, 0));
    vec<t,n> Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret[I] = L[I] / R; 
    }
    return Ret;
}


template<typename t, usize n>
static inline vec<t,n> 
Negate(vec<t,n> V){
    vec<t,n> Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret[I] = -V; 
    }
    return Ret;
}

template<typename t, usize n>
static inline b32 
IsEqual(vec<t,n> L, vec<t,n> R) {
    for (usize I = 0; I < n; ++I) {
        if (!IsEqual(L[I], R[I])) {
            return false;
        }
    }
    return true;
}

template<typename t, usize n>
static inline t 
Dot(vec<t,n> L, vec<t,n> R) {
    t Ret = {};
    for (usize I = 0; I < n; ++I) {
        Ret += L[I] * R[I]; 
    }
    return Ret;
}

template<typename t, usize n>
static inline vec<t,n> 
operator+(vec<t,n> L, vec<t,n> R)  { 
    return Add(L, R); 
}

template<typename t, usize n>
static inline vec<t,n> 
operator-(vec<t,n> L, vec<t,n> R)  { 
    return Sub(L, R);
}

template<typename t, usize n>
static inline vec<t,n> 
operator*(vec<t,n> L, t R)  { 
    return Mul(L, R);
}

template<typename t, usize n>
static inline vec<t,n> 
operator*(t L, vec<t,n> R)  { 
    return Mul(R, L);
}

template<typename t, usize n>
static inline t 
operator*(vec<t,n> L, vec<t,n> R) {
    return Dot(L, R); 
}


template<typename t, usize n>
static inline vec<t,n> 
operator/(vec<t,n> L, t R)  { 
    return Div(L, R); 
}


template<typename t, usize n>
static inline vec<t,n>& 
operator+=(vec<t,n>& L, vec<t,n> R) {
    return L = L + R;
}


template<typename t, usize n>
static inline vec<t,n>& 
operator-=(vec<t,n>& L, vec<t,n> R) {
    return L = L - R;
}


template<typename t, usize n>
static inline vec<t,n>& 
operator*=(vec<t,n>& L, t R) {
    return L = L * R;
}


template<typename t, usize n>
static inline vec<t,n>& 
operator/=(vec<t,n>& L, t R) {
    return L = L / R;
}


template<typename t, usize n>
static inline b32 
operator==(vec<t,n> L, vec<t,n> R)  { 
    return IsEqual(L, R);
}


template<typename t, usize n>
static inline b32 
operator!=(vec<t,n> L, vec<t,n> R) { 
    return !(L == R); 
}


template<typename t, usize n>
static inline vec<t,n> 
operator-(vec<t,n> V)  { 
    return Negate(V); 
}

template<typename t, usize n>
static inline vec<t,n> 
Midpoint(vec<t,n> L, vec<t,n> R)  { 
    return (L + R) / (t)2.f; 
}

template<typename t, usize n>
static inline t
DistanceSq(vec<t,n> L, vec<t,n> R) {
    t Ret = {};
    for (usize I = 0; I < n; ++I ){
        Ret += (R[I] - L[I]) * (R[I] - L[I]);
    }
    return Ret;
}

template<typename t, usize n>
static inline t
LengthSq(vec<t,n> V) { 
    return V * V;
}

template<typename t, usize n>
static inline t 
Distance(vec<t,n> L, vec<t,n> R)  { 
    return Sqrt(DistanceSq(L, R)); 
}

template<typename t, usize n>
static inline t 
Length(vec<t,n> L)  { 
    return Sqrt(LengthSq(L));
}

template<typename t, usize n>
static inline vec<t,n> 
Normalize(vec<t,n> V)  {
    vec<t,n> Ret = V;
    Ret /= Length(V);
    return Ret;
}

template<typename t, usize n>
static inline t 
AngleBetween(vec<t,n> L, vec<t,n> R) {
    return ACos((L * R) / (Length(L) * Length(R)));
}


template<typename t, usize n>
static inline b32
IsPerpendicular(vec<t,n> L, vec<t,n> R) { 
    return IsEqual((L * R), 0); 
}


template<typename t, usize n>
static inline b32 
IsSameDir(vec<t,n> L, vec<t,n> R) { 
    return (L * R) > 0; 
}


template<typename t, usize n>
static inline b32 
IsOppDir(vec<t,n> L, vec<t,n> R) { 
    return (L * R) < 0;
}

template<typename t, usize n>
static inline vec<t,n> 
Project(vec<t,n> From, vec<t,n> To) { 
    return (To * From) / LengthSq(To) * To;
}

template<typename t, usize n>
static inline b32
IsIntersecting(circle<t,n> L, circle<t,n> R) {
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

// Constructors...
static inline v2f 
V2f(v3f V) {
    return { V.X, V.Y };
}

static inline v3f 
V3f(v2f V) {
    return { V.X, V.Y, 0.f };
}

static inline v2f
V2f(v2u V) {
    return { (f32)V.X, (f32)V.Y };
}

static inline v2f
V2f(v2i V) {
    return { (f32)V.X, (f32)V.Y };
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


static inline aabb2f
Aabb2f(aabb3f Aabb) {
    return {
        Aabb.Min.XY,
        Aabb.Max.XY,
    };
}

static inline aabb2f 
Aabb2f(aabb2u Aabb) {
    return {
        V2f(Aabb.Min),
        V2f(Aabb.Max),
    };
}

static inline aabb2f 
Aabb2f(aabb2i Aabb) {
    return {
        V2f(Aabb.Min),
        V2f(Aabb.Max),
    };
}

static inline aabb3f
Aabb3f(aabb2f Aabb) {
    return {
        V3f(Aabb.Min),
        V3f(Aabb.Max),
    };
}

static inline aabb3f
CenteredAabb3f(v3f Dimensions, v3f Anchor) {
    aabb3f Ret = {};
    Ret.Min.X = Lerp(0.f, -Dimensions.W, Anchor.X);
    Ret.Max.X = Lerp(Dimensions.W, 0.f, Anchor.X);
    
    Ret.Min.Y = Lerp(0.f, -Dimensions.H, Anchor.Y);
    Ret.Max.Y = Lerp(Dimensions.H, 0.f, Anchor.Y);
    
    Ret.Min.Z = Lerp(0.f, -Dimensions.D, Anchor.Z);
    Ret.Max.Z = Lerp(Dimensions.D, 0.f, Anchor.Z);
    
    return Ret; 
}

static inline f32 
AspectRatio(aabb2f R) {
    return GetWidth(R)/GetHeight(R);
}

static inline f32
AspectRatio(aabb2u R) {
    return (f32)GetWidth(R)/GetHeight(R);
}


// NOTE(Momo): Gets the Normalized values of Aabb A based on another Aabb B
static inline aabb2f 
Ratio(aabb2f A, aabb2f B) {
    return  {
        Ratio(A.Min.X, B.Min.X, B.Max.X),
        Ratio(A.Min.Y, B.Min.Y, B.Max.Y),
        Ratio(A.Max.X, B.Min.X, B.Max.X),
        Ratio(A.Max.Y, B.Min.X, B.Max.Y),
    };
}

static inline aabb2f 
Ratio(aabb2u A, aabb2u B) {
    return Ratio(Aabb2f(A), Aabb2f(B));
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
Quad2f(aabb2f Aabb) {
    return quad2f{
        Aabb.Min.X, Aabb.Max.Y, // top left
        Aabb.Max.X, Aabb.Max.Y, // top right
        Aabb.Max.X, Aabb.Min.Y, // bottom right
        Aabb.Min.X, Aabb.Min.Y, // bottom left
    };
}

#endif 
