#ifndef __RYOJI_MATH__
#define __RYOJI_MATH__

#include "ryoji.h"

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

// NOTE(Momo): [-PI,PI]
static inline f32 
Sin(f32 x) {
    constexpr float B = 4.0f / Pi32;
    constexpr float C = -4.0f / (Pi32 * Pi32);
    constexpr float P = 0.225f;
    
    float y = B * x + C * x * Abs(x);
    return P * (y * Abs(y) - y) + y;
}

// NOTE(Momo): [-PI, PI]
static inline 
f32 Cos(f32 x) {
    constexpr float B = 4.0f / Pi32;
    constexpr float C = -4.0f / (Pi32 * Pi32);
    constexpr float P = 0.225f;
    
    x = (x > 0) ? -x : x;
    x += Pi32/2;
    
    return Sin(x);
}

// NOTE(Momo): [-PI, PI]
static inline f32 
Tan(f32 x) {
    return Sin(x)/Cos(x);
}


static inline f32 
InvSqrt(f32 number)
{
    u32 i;
	f32 x2, y;
	f32 threehalfs = 1.5f;
    
	x2 = number * 0.5F;
	y  = number;
	i  = *(u32*) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = *(f32*) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
    //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
    
	return y;
}

static inline f32 
Sqrt(f32 number) {
    return 1.f/InvSqrt(number);
}



// NOTE(Momo): [-1, 1]
static inline f32 
ASin(f32 x) {
    f32 negate = f32(x < 0);
    x = Abs(x);
    f32 ret = -0.0187293f;
    ret *= x;
    ret += 0.0742610f;
    ret *= x;
    ret -= 0.2121144f;
    ret *= x;
    ret += 1.5707288f;
    ret = Pi32 *0.5f - Sqrt(1.0f - x)*ret;
    return ret - 2 * negate * ret;
}



// NOTE(Momo): [-1, 1]
static inline f32 
ACos(f32 x) {
    f32 negate = f32(x < 0);
    x = Abs(x);
    f32 ret = -0.0187293f;
    ret = ret * x;
    ret = ret + 0.0742610f;
    ret = ret * x;
    ret = ret - 0.2121144f;
    ret = ret * x;
    ret = ret + 1.5707288f;
    ret = ret * Sqrt(1.f-x);
    ret = ret - 2.f * negate * ret;
    return negate * Pi32 + ret;
}

// NOTE(Momo): [-1, 1]
static inline f32 
ATan(f32 x) {
    constexpr f32 A = 0.0776509570923569f;
    constexpr f32 B = -0.287434475393028f;
    constexpr f32 C = Pi32 / 4 - A - B;
    f32 xx = x * x;
    return ((A*xx + B)*xx + C)*x;
}




// NOTE(Momo): Vectors
struct v3f {
    union {
        f32 arr[3];
        struct {
            f32 x;
            f32 y;
            f32 z;
        };	
    };
    
    inline f32 operator[](usize index) const { 
        return arr[index]; 
    } 
};

static inline v3f 
Add(v3f lhs, v3f rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    
}

static inline v3f 
Sub(v3f lhs, v3f rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

static inline v3f 
Mul(v3f lhs, f32 rhs) {
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

static inline v3f 
Div(v3f lhs, f32 rhs) {
    Assert(IsEqual(rhs, 0.f));
    return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

static inline v3f 
Negate(v3f lhs){
    return {-lhs.x, -lhs.y, -lhs.z};
    
}
static inline bool 
IsEqual(v3f lhs, v3f rhs) {
    return 
        IsEqual(lhs.x, rhs.x) && 
        IsEqual(lhs.y, rhs.y) && 
        IsEqual(lhs.z, rhs.z);
}

static inline f32 
Dot(v3f lhs, v3f rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
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
    return { -lhs.x, -lhs.y, -lhs.z}; 
}

static inline v3f 
Midpoint(v3f lhs, v3f rhs)  { 
    return (lhs + rhs)/2.f; 
}

static inline f32 
DistSq(v3f lhs, v3f rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
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
    f32 Arr[4][4];
    
    inline const auto& operator[](usize index) const { return Arr[index]; }
    inline auto& operator[](usize index) { return Arr[index];}
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
Translation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, x,
        0.f, 1.f, 0.f, y,
        0.f, 0.f, 1.f, z,
        0.f, 0.f, 0.f, 1.f
    };
}


static inline m44f 
RotationX(f32 rad) {
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
RotationY(f32 rad) {
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
RotationZ(f32 rad) {
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
Scale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f,   0.f,
        0.f, y, 0.f,   0.f,
        0.f, 0.f, z,   0.f,
        0.f, 0.f, 0.f, 1.f
    };
}


// 
static inline m44f 
Orthographic(f32 NdcLeft, f32 NdcRight,
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
Identity() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}




#endif 