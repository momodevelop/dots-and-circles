#ifndef __MOMO_MATH__
#define __MOMO_MATH__



// NOTE(Momo): To future self: We wrap the math functions IN CASE we 
// want to have specialized versions for different systems (using SSE etc)
//
// I guess a good rule of thumb is that the GAME CODE should never directly
// use standard library functions IN CASE we cannot use them!
//

// NOTE(Momo): Constants
static const f32 Pi32 = 3.14159265358979323846264338327950288f;
static const f32 Epsilon32  = 1.19209290E-07f;
static const f32 Tau32  = Pi32 * 2.f;

#define GenerateSubscriptOp(Amt) inline auto& operator[](usize I) { Assert(I < Amt); return Elements[I]; }



// NOTE(Momo): Common Functions
static inline b32
F32_IsEqual(f32 L, f32 R) {
    return AbsOf(L - R) <= Epsilon32;
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

//~ NOTE(Momo): Vectors
struct v2f {
    union {
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
    };
    GenerateSubscriptOp(2)
};

struct v2u {
    union {
        u32 Elements[2];
        struct {
            u32 X;
            u32 Y;
        };
        struct {
            u32 U;
            u32 V;
        };
        struct {
            u32 W;
            u32 H;
        };
    };
    GenerateSubscriptOp(2)
};

struct v2i {
    union {
        i32 Elements[2];
        struct {
            i32 X;
            i32 Y;
        };
        struct {
            i32 U;
            i32 V;
        };
        struct {
            i32 W;
            i32 H;
        };
    };
    GenerateSubscriptOp(2)
};

struct v3f {
    union {
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
    };
    GenerateSubscriptOp(3)
};

struct v4f {
    union {
        f32 E[4];
        struct {
            union {
                v3f XYZ;
                struct {
                    f32 X, Y, Z;
                };
            }; 
            f32 W;
        };
    };
};

//~ NOTE(Momo): v2f functions
static inline v2f 
V2f_Add(v2f L, v2f R) {
    L.X += R.X;
    L.Y += R.Y;
    return L;
}

static inline v2f
V2f_Sub(v2f L, v2f R) {
    L.X -= R.X;
    L.Y -= R.Y;
    return L;
}


static inline v2f
V2f_Mul(v2f L, f32 R) {
    L.X *= R;
    L.Y *= R;
    return L;
}


static inline v2f
V2f_Div(v2f L, f32 R) {
    Assert(!F32_IsEqual(R, 0));
    L.X /= R;
    L.Y /= R;
    
    return L;
}

static inline v2f
V2f_Neg(v2f V){
    V.X = -V.X;
    V.Y = -V.Y;
    return V;
}

static inline b8 
V2f_IsEqual(v2f L, v2f R) {
    return 
        F32_IsEqual(L.X, R.X) && 
        F32_IsEqual(L.Y, R.Y);
}

static inline f32 
V2f_Dot(v2f L, v2f R) {
    f32 Ret = {};
    Ret = L.X * R.X + L.Y * R.Y;
    return Ret;
}

static inline v2f 
V2f_Midpoint(v2f L, v2f R)  { 
    v2f LR = V2f_Add(L, R);
    v2f Ret = V2f_Div(LR, 2.f);
    return Ret; 
}

static inline f32
V2f_LengthSq(v2f V) { 
    // NOTE(Momo): Dot Product trick!
    return V2f_Dot(V, V);
}

static inline f32
V2f_DistanceSq(v2f L, v2f R) {
    v2f V = V2f_Sub(R, L);
    f32 Ret = V2f_LengthSq(V); 
    return Ret;
}

static inline f32
V2f_Distance(v2f L, v2f R)  { 
    return Sqrt(V2f_DistanceSq(L, R)); 
}

static inline f32 
V2f_Length(v2f L)  { 
    return Sqrt(V2f_LengthSq(L));
}

static inline v2f 
V2f_Normalize(v2f V)  {
    f32 Len = V2f_Length(V);
    v2f Ret = V2f_Div(V, V2f_Length(V));
    return Ret;
}

static inline f32
V2f_AngleBetween(v2f L, v2f R) {
    f32 LLen = V2f_Length(L);
    f32 RLen = V2f_Length(R);
    f32 LRDot = V2f_Dot(L,R);
    f32 Ret = ACos(LRDot/(LLen * RLen));
    
    return Ret;
}


static inline b32
V2f_IsPerpendicular(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return F32_IsEqual(LRDot, 0); 
}


static inline b32 
V2f_IsSameDir(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return LRDot > 0; 
}


static inline b32 
V2f_IsOppDir(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return LRDot < 0;
}

static inline v2f 
V2f_Projection(v2f From, v2f To) { 
    // (To . From)/LenSq(To) * To
    f32 ToLenSq = V2f_LengthSq(To);
    Assert(!F32_IsEqual(ToLenSq, 0)); 
    
    f32 ToDotFrom = V2f_Dot(To, From);
    f32 UnitProjectionScalar = ToDotFrom / ToLenSq;
    v2f Ret = V2f_Mul(To, UnitProjectionScalar);
    return Ret;
}


//~ NOTE(Momo): v3f Functions
static inline v3f 
V3f_Add(v3f L, v3f R) {
    L.X += R.X;
    L.Y += R.Y;
    L.Z += R.Z;
    return L;
}

static inline v3f
V3f_Sub(v3f L, v3f R) {
    L.X -= R.X;
    L.Y -= R.Y;
    L.Z -= R.Z;
    return L;
}


static inline v3f
V3f_Mul(v3f L, f32 R) {
    L.X *= R;
    L.Y *= R;
    L.Z *= R;
    return L;
}


static inline v3f
V3f_Div(v3f L, f32 R) {
    Assert(!F32_IsEqual(R, 0));
    L.X /= R;
    L.Y /= R;
    L.Z /= R;
    return L;
}

static inline v3f
V3f_Neg(v3f V){
    V.X = -V.X;
    V.Y = -V.Y;
    V.Z = -V.Z;
    return V;
}

static inline b8 
V3f_IsEqual(v3f L, v3f R) {
    return 
        F32_IsEqual(L.X, R.X) && 
        F32_IsEqual(L.Y, R.Y) &&
        F32_IsEqual(L.Z, R.Z);
}

static inline f32 
V3f_Dot(v3f L, v3f R) {
    f32 Ret = {};
    Ret = L.X * R.X + L.Y * R.Y + L.Z * R.Z;
    return Ret;
}

static inline v3f 
V3f_Midpoint(v3f L, v3f R)  { 
    v3f LR = V3f_Add(L, R);
    v3f Ret = V3f_Div(LR, 2.f);
    return Ret; 
}

static inline f32
V3f_LengthSq(v3f V) { 
    // NOTE(Momo): Dot Product trick!
    return V3f_Dot(V, V);
}

static inline f32
V3f_DistanceSq(v3f L, v3f R) {
    v3f V = V3f_Sub(R, L);
    f32 Ret = V3f_LengthSq(V); 
    return Ret;
}

static inline f32
V3f_Distance(v3f L, v3f R)  { 
    return Sqrt(V3f_DistanceSq(L, R)); 
}

static inline f32 
V3f_Length(v3f L)  { 
    return Sqrt(V3f_LengthSq(L));
}

static inline v3f 
V3f_Normalize(v3f V)  {
    f32 Len = V3f_Length(V);
    v3f Ret = V3f_Div(Ret, V3f_Length(V));
    return Ret;
}

static inline f32
V3f_AngleBetween(v3f L, v3f R) {
    f32 LLen = V3f_Length(L);
    f32 RLen = V3f_Length(R);
    f32 LRDot = V3f_Dot(L,R);
    f32 Ret = ACos(LRDot/(LLen * RLen));
    
    return Ret;
}


static inline b32
V3f_IsPerpendicular(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return F32_IsEqual(LRDot, 0); 
}


static inline b32 
V3f_IsSameDir(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return LRDot > 0; 
}


static inline b32 
V3f_IsOppDir(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return LRDot < 0;
}

static inline v3f 
V3f_Projection(v3f From, v3f To) { 
    // (To . From)/LenSq(To) * To
    f32 ToLenSq = V3f_LengthSq(To);
    Assert(!F32_IsEqual(ToLenSq, 0)); 
    
    f32 ToDotFrom = V3f_Dot(To, From);
    f32 UnitProjectionScalar = ToDotFrom / ToLenSq;
    v3f Ret = V3f_Mul(To, UnitProjectionScalar);
    return Ret;
}

static inline v3f 
V2f_To_V3f(v2f V) {
    return { V.X, V.Y, 0.f };
}

static inline v2f
V2u_To_V2f(v2u V) {
    return { (f32)V.X, (f32)V.Y };
}

static inline v2f 
V2i_To_V2f(v2i V) {
    return { (f32)V.X, (f32)V.Y };
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

static inline aabb2f
Aabb2f_Mul(aabb2f Lhs, f32 Rhs) {
    Lhs.Min = V2f_Mul(Lhs.Min, Rhs);
    Lhs.Max = V2f_Mul(Lhs.Max, Rhs);
    return Lhs;
}


static inline f32
Aabb2f_Width(aabb2f Aabb) {
    return Aabb.Max.X - Aabb.Min.X;
}

static inline f32
Aabb2f_Height(aabb2f Aabb) {
    return Aabb.Max.Y - Aabb.Min.Y; 
}


static inline u32
Aabb2u_Width(aabb2u Aabb) {
    return Aabb.Max.X - Aabb.Min.X;
}

static inline u32
Aabb2u_Height(aabb2u Aabb) {
    return Aabb.Max.Y - Aabb.Min.Y; 
}

static inline aabb2u
Aabb2u_CreateXYWH(u32 X, u32 Y, u32 Width, u32 Height) {
    aabb2u Ret = {};
    Ret.Min.X = X;
    Ret.Min.Y = Y;
    Ret.Max.X = X + Width;
    Ret.Max.Y = Y + Height;
    return Ret;
}

static inline aabb3f
Aabb3f_Centered(v3f Dimensions, v3f Anchor) {
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
Aabb2f_AspectRatio(aabb2f V) {
    return (f32)Aabb2f_Width(V)/Aabb2f_Height(V);
}

static inline aabb2f
Aabb2i_To_Aabb2f(aabb2i V) {
    aabb2f Ret = {};
    Ret.Min = V2i_To_V2f(V.Min);
    Ret.Max = V2i_To_V2f(V.Max);
    return Ret;
}

static inline aabb2f
Aabb2u_To_Aabb2f(aabb2u V) {
    aabb2f Ret = {};
    Ret.Min = V2u_To_V2f(V.Min);
    Ret.Max = V2u_To_V2f(V.Max);
    return Ret;
}

// NOTE(Momo): Gets the Normalized values of Aabb A based on another Aabb B
static inline aabb2f 
Aabb2f_Ratio(aabb2f A, aabb2f B) {
    return  {
        Ratio(A.Min.X, B.Min.X, B.Max.X),
        Ratio(A.Min.Y, B.Min.Y, B.Max.Y),
        Ratio(A.Max.X, B.Min.X, B.Max.X),
        Ratio(A.Max.Y, B.Min.X, B.Max.Y),
    };
}

static inline aabb2f 
Aabb2u_Ratio(aabb2u A, aabb2u B) {
    aabb2f Af = Aabb2u_To_Aabb2f(A);
    aabb2f Bf = Aabb2u_To_Aabb2f(B);
    return Aabb2f_Ratio(Af, Bf);
}

//~ NOTE(Momo): Circles
struct circle2f {
    v2f Origin;
    f32 Radius;
};

static inline b32
Circle2f_IsIntersecting(circle2f L, circle2f R) {
	f32 DistSq = V2f_DistanceSq(L.Origin, R.Origin);
	f32 RSq = L.Radius + R.Radius;
    RSq *= RSq;
	return DistSq < RSq;
}

//~ NOTE(Momo): Matrices
//
union m44f {
    f32 E[16];
    f32 EE[4][4];
};

static inline m44f 
M44f_Concat(m44f L, m44f R) {
    m44f Ret = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                Ret.EE[r][c] += L.EE[r][i] *  R.EE[i][c]; 
        } 
    } 
    return Ret;
}

static inline m44f 
M44f_Transpose(m44f M) {
    m44f Ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            Ret.EE[i][j] = M.EE[j][i];
        }
    }
    
    return Ret;
}


static inline m44f 
M44f_Translation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}

static inline m44f 
M44f_RotationX(f32 rad) {
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
M44f_RotationY(f32 rad) {
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
M44f_RotationZ(f32 rad) {
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
M44f_Scale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
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
    Ret.EE[0][0] = (NdcRight-NdcLeft)/(Right-Left);
    Ret.EE[1][1] = (NdcTop-NdcBottom)/(Top-Bottom);
    Ret.EE[2][2] = (FlipZ ? -1.f : 1.f) * (NdcFar-NdcNear)/(Far-Near);
    Ret.EE[3][3] = 1.f;
    Ret.EE[0][3] = -(Right+Left)/(Right-Left);
    Ret.EE[1][3] = -(Top+Bottom)/(Top-Bottom);
    Ret.EE[2][3] = -(Far+Near)/(Far-Near);
    
    return Ret;
}


static inline m44f 
M44f_Identity() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

//~ NOTE(Momo): Ray and lines
struct line2f {
    v2f Min;
    v2f Max;
};

struct ray2f {
    v2f Origin;
    v2f Direction;
};

static inline ray2f
Line2f_To_Ray2f(line2f Line) {
    ray2f Ret = {};
    Ret.Origin = Line.Min;
    Ret.Direction = V2f_Sub(Line.Max, Line.Min);
    return Ret;
}

static inline void 
Ray2f_IntersectionTime(ray2f Lhs, ray2f Rhs, f32* LhsTimeResult, f32* RhsTimeResult) {
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
Ray2f_Point(ray2f Ray, f32 Time) {
    // O + D * T
    v2f DirTime = V2f_Mul(Ray.Direction, Time);
    v2f Ret = V2f_Add(Ray.Origin, DirTime);
    return Ret;
}

// Quad
struct quad2f {
    v2f Points[4];
    inline auto& operator[](usize I) { 
        Assert(I < 4);
        return Points[I];
    }
    
};


static inline quad2f
Aabb2f_To_Quad2f(aabb2f Aabb) {
    return quad2f{
        Aabb.Min.X, Aabb.Max.Y, // top left
        Aabb.Max.X, Aabb.Max.Y, // top right
        Aabb.Max.X, Aabb.Min.Y, // bottom right
        Aabb.Min.X, Aabb.Min.Y, // bottom left
    };
}

#endif 
