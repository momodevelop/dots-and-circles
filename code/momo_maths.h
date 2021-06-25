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

#define GenerateSubscriptOp(Amt) inline auto& operator[](u32 I) { Assert(I < Amt); return Elements[I]; }

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
        s32 Elements[2];
        struct {
            s32 X;
            s32 Y;
        };
        struct {
            s32 U;
            s32 V;
        };
        struct {
            s32 W;
            s32 H;
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
        f32 Elements[4];
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
    GenerateSubscriptOp(4);
};

//~ NOTE(Momo): v2f functions
static inline v2f
V2f_Create(f32 X, f32 Y) {
    v2f Ret = {};
    Ret.X = X;
    Ret.Y = Y;
    
    return Ret;
}

static inline v2f
V2f_CreateFromV2u(v2u V) {
    return V2f_Create((f32)V.X, (f32)V.Y);
}

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
    Assert(!is_equal(R, 0));
    L.X /= R;
    L.Y /= R;
    
    return L;
}

static inline v2f
V2f_Ratio(v2f L, v2f R) {
    Assert(!is_equal(R.X, 0));
    Assert(!is_equal(R.Y, 0));
    
    L.X /= R.X;
    L.Y /= R.Y;
    
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
        is_equal(L.X, R.X) && 
        is_equal(L.Y, R.Y);
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
    return sqrt(V2f_DistanceSq(L, R)); 
}

static inline f32 
V2f_Length(v2f L)  { 
    return sqrt(V2f_LengthSq(L));
}

static inline v2f 
V2f_Normalize(v2f V)  {
    f32 Len = V2f_Length(V);
    v2f Ret = V2f_Div(V, Len);
    return Ret;
}

static inline f32
V2f_AngleBetween(v2f L, v2f R) {
    f32 LLen = V2f_Length(L);
    f32 RLen = V2f_Length(R);
    f32 LRDot = V2f_Dot(L,R);
    f32 Ret = acos(LRDot/(LLen * RLen));
    
    return Ret;
}

static inline b8
V2f_IsPerp(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return is_equal(LRDot, 0); 
}


static inline b8 
V2f_IsSameDir(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return LRDot > 0; 
}


static inline b8 
V2f_IsOppDir(v2f L, v2f R) { 
    f32 LRDot = V2f_Dot(L,R);
    return LRDot < 0;
}

static inline v2f 
V2f_Project(v2f From, v2f To) { 
    // (To . From)/LenSq(To) * To
    f32 ToLenSq = V2f_LengthSq(To);
    Assert(!is_equal(ToLenSq, 0)); 
    
    f32 ToDotFrom = V2f_Dot(To, From);
    f32 UnitProjectionScalar = ToDotFrom / ToLenSq;
    v2f Ret = V2f_Mul(To, UnitProjectionScalar);
    return Ret;
}

static inline v2f
V2f_Rotate(v2f V, f32 Rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 C = cos(Rad);
    f32 S = sin(Rad);
    
    v2f Ret = {};
    Ret.X = (C * V.X) - (S * V.Y);
    Ret.Y = (S * V.X) + (C * V.Y);
    return Ret;
}

static inline v2f 
operator+(v2f Lhs, v2f Rhs) {
    return V2f_Add(Lhs, Rhs);
}

static inline v2f&
operator+=(v2f& Lhs, v2f Rhs) {
    return Lhs = Lhs + Rhs;
}

static inline v2f 
operator-(v2f Lhs, v2f Rhs) {
    return V2f_Sub(Lhs, Rhs);
}

static inline v2f&
operator-=(v2f& Lhs, v2f Rhs) {
    return Lhs = Lhs - Rhs;
}

static inline v2f 
operator*(v2f Lhs, f32 Rhs) {
    return V2f_Mul(Lhs, Rhs);
}

static inline v2f 
operator*=(v2f Lhs, f32 Rhs) {
    return Lhs = Lhs * Rhs;
}

static inline v2f 
operator*(f32 Lhs, v2f Rhs) {
    return V2f_Mul(Rhs, Lhs);
}

//~ NOTE(Momo): v2u Functions
static inline v2u 
V2u_Create(u32 X, u32 Y) {
    v2u Ret = {};
    Ret.X = X;
    Ret.Y = Y;
    
    return Ret;
}

//~ NOTE(Momo): v3f Functions
static inline v3f
V3f_Create(f32 X, f32 Y, f32 Z) {
    v3f Ret = {};
    Ret.X = X;
    Ret.Y = Y;
    Ret.Z = Z;
    return Ret;
}

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
    Assert(!is_equal(R, 0));
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
        is_equal(L.X, R.X) && 
        is_equal(L.Y, R.Y) &&
        is_equal(L.Z, R.Z);
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
    return sqrt(V3f_DistanceSq(L, R)); 
}

static inline f32 
V3f_Length(v3f L)  { 
    return sqrt(V3f_LengthSq(L));
}

static inline v3f 
V3f_Normalize(v3f V)  {
    f32 Len = V3f_Length(V);
    v3f Ret = V3f_Div(V, Len);
    return Ret;
}

static inline f32
V3f_AngleBetween(v3f L, v3f R) {
    f32 LLen = V3f_Length(L);
    f32 RLen = V3f_Length(R);
    f32 LRDot = V3f_Dot(L,R);
    f32 Ret = acos(LRDot/(LLen * RLen));
    
    return Ret;
}


static inline b8
V3f_IsPerpendicular(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return is_equal(LRDot, 0); 
}


static inline b8 
V3f_IsSameDir(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return LRDot > 0; 
}


static inline b8 
V3f_IsOppDir(v3f L, v3f R) { 
    f32 LRDot = V3f_Dot(L,R);
    return LRDot < 0;
}

static inline v3f 
V3f_Project(v3f From, v3f To) { 
    // (To . From)/LenSq(To) * To
    f32 ToLenSq = V3f_LengthSq(To);
    Assert(!is_equal(ToLenSq, 0)); 
    
    f32 ToDotFrom = V3f_Dot(To, From);
    f32 UnitProjectionScalar = ToDotFrom / ToLenSq;
    v3f Ret = V3f_Mul(To, UnitProjectionScalar);
    return Ret;
}

static inline v3f 
V2f_To_V3f(v2f V) {
    v3f Ret = {};
    Ret.X = V.X;
    Ret.Y = V.Y;
    Ret.Z = 0.f;
    
    return Ret;
}

static inline v2f
V2u_To_V2f(v2u V) {
    v2f Ret = {};
    Ret.X = (f32)V.X;
    Ret.Y = (f32)V.Y;
    
    return Ret;
}

static inline v2f 
V2i_To_V2f(v2i V) {
    v2f Ret = {};
    Ret.X = (f32)V.X;
    Ret.Y = (f32)V.Y;
    
    return Ret;
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
    v2u Min = V2u_Create(MinX, MinY);
    v2u Max = V2u_Create(MaxX, MaxY);
    
    return Aabb2u_CreateFromV2u(Min, Max);
    
}


static inline u32
Aabb2u_Width(aabb2u A) {
    return A.Max.X - A.Min.X;
}

static inline u32
Aabb2u_Height(aabb2u A) {
    return A.Max.Y - A.Min.Y; 
}

static inline v2u
Aabb2u_Dimensions(aabb2u A) {
    v2u Ret = {};
    Ret.W = Aabb2u_Width(A);
    Ret.H = Aabb2u_Height(A);
    
    return Ret;
}

static inline aabb2f
Aabb2f_Mul(aabb2f Lhs, f32 Rhs) {
    Lhs.Min = V2f_Mul(Lhs.Min, Rhs);
    Lhs.Max = V2f_Mul(Lhs.Max, Rhs);
    return Lhs;
}

static inline f32
Aabb2f_Width(aabb2f A) {
    return A.Max.X - A.Min.X;
}

static inline f32
Aabb2f_Height(aabb2f A) {
    return A.Max.Y - A.Min.Y; 
}


static inline v2f
Aabb2f_Dimensions(aabb2f A) {
    v2f Ret = {};
    Ret.W = Aabb2f_Width(A);
    Ret.H = Aabb2f_Height(A);
    
    return Ret;
}


static inline aabb2u
Aabb2u_CreateXYWH(u32 X, u32 Y, u32 W, u32 H) {
    aabb2u Ret = {};
    Ret.Min.X = X;
    Ret.Min.Y = Y;
    Ret.Max.X = X + W;
    Ret.Max.Y = Y + H;
    return Ret;
}

static inline aabb2u
Aabb2u_CreateWH(u32 W, u32 H) {
    aabb2u Ret = {};
    Ret.Max.X = W;
    Ret.Max.Y = H;
    
    return Ret;
}


static inline aabb3f
Aabb3f_CreateCentered(v3f Dimensions, v3f Anchor) {
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
    aabb2f Ret = {};
    Ret.Min.X = ratio(A.Min.X, B.Min.X, B.Max.X);
    Ret.Min.Y = ratio(A.Min.Y, B.Min.Y, B.Max.Y);
    Ret.Max.X = ratio(A.Max.X, B.Min.X, B.Max.X);
    Ret.Max.Y = ratio(A.Max.Y, B.Min.X, B.Max.Y);
    
    return Ret;
}

static inline aabb2f 
Aabb2u_Ratio(aabb2u A, aabb2u B) {
    aabb2f Af = Aabb2u_To_Aabb2f(A);
    aabb2f Bf = Aabb2u_To_Aabb2f(B);
    return Aabb2f_Ratio(Af, Bf);
}

static inline aabb2u
Aabb2u_Translate(aabb2u V, u32 X, u32 Y) {
    aabb2u Ret = V;
    Ret.Min.X += X;
    Ret.Min.Y += Y;
    Ret.Max.X += X;
    Ret.Max.Y += Y;
    
    return Ret;
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
    Lhs.Origin = V2f_Add(Lhs.Origin, Offset);
    return Lhs;
}

//~ NOTE(Momo): 2x2 Matrices
union m22f {
    v2f Elements[4];
    GenerateSubscriptOp(2);
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
    v4f Elements[4];
    GenerateSubscriptOp(4);
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
    v2f Min = V2f_Create(MinX, MinY);
    v2f Max = V2f_Create(MaxX, MaxY);
    
    return Line2f_CreateFromV2f(Min, Max);
    
}

static inline ray2f
Ray2f_CreateFromLine2f(line2f L) {
    ray2f Ret = {};
    Ret.Origin = L.Min;
    Ret.Dir = V2f_Sub(L.Max, L.Min);
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
    
    
    t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
    t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
    
    *LhsTimeResult = t1;
    *RhsTimeResult = t2;
}

static inline v2f
Ray2f_Point(ray2f Ray, f32 Time) {
    // O + D * T
    v2f DirTime = V2f_Mul(Ray.Dir, Time);
    v2f Ret = V2f_Add(Ray.Origin, DirTime);
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
    Ret.Points[0] = V2f_Create(0.f, 1.f);
    Ret.Points[1] = V2f_Create(1.f, 1.f);
    Ret.Points[2] = V2f_Create(1.f, 0.f);
    Ret.Points[3] = V2f_Create(0.f, 0.f);
    
    return Ret;
}


static inline quad2f
Aabb2f_To_Quad2f(aabb2f Aabb) {
    quad2f Ret = {};
    Ret.Points[0] = V2f_Create(Aabb.Min.X, Aabb.Max.Y);
    Ret.Points[1] = V2f_Create(Aabb.Max.X, Aabb.Max.Y);
    Ret.Points[2] = V2f_Create(Aabb.Max.X, Aabb.Min.Y);
    Ret.Points[3] = V2f_Create(Aabb.Min.X, Aabb.Min.Y);
    
    return Ret;
}


#undef GenerateSubscriptOp
#endif 
