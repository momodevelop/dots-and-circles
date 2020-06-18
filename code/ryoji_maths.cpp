#ifndef __RYOJI_MATH__
#define __RYOJI_MATH__

#include "ryoji.cpp"

global constexpr f32 PIf = 3.14159265358979323846264338327950288f;
global constexpr f32 EPSILONf = 1.19209290E-07f;

pure bool IsEqual(f32 lhs, f32 rhs) {
    return Abs(lhs - rhs) <= EPSILONf;
}

// Degrees and Radians
pure f32 DegToRad(f32 degrees) {
    return degrees * PIf / 180.f;
}

pure f32 RadToDeg(f32 radians) {
    return radians * 180.f / PIf;
}

// NOTE(Momo): [-PI,PI]
pure f32 Sin(f32 x) {
    constexpr float B = 4.0f / PIf;
    constexpr float C = -4.0f / (PIf * PIf);
    constexpr float P = 0.225f;
    
    float y = B * x + C * x * Abs(x);
    return P * (y * Abs(y) - y) + y;
}

// NOTE(Momo): [-PI, PI]
pure f32 Cos(f32 x) {
    constexpr float B = 4.0f / PIf;
    constexpr float C = -4.0f / (PIf * PIf);
    constexpr float P = 0.225f;
    
    x = (x > 0) ? -x : x;
    x += PIf/2;
    
    return Sin(x);
}

// NOTE(Momo): [-PI, PI]
pure f32 Tan(f32 x) {
    return Sin(x)/Cos(x);
}


pure f32 InvSqrt(f32 number)
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

pure f32 Sqrt(f32 number) {
    return 1.f/InvSqrt(number);
}



// NOTE(Momo): [-1, 1]
pure f32 ASin(f32 x) {
    f32 negate = f32(x < 0);
    x = Abs(x);
    f32 ret = -0.0187293f;
    ret *= x;
    ret += 0.0742610f;
    ret *= x;
    ret -= 0.2121144f;
    ret *= x;
    ret += 1.5707288f;
    ret = PIf *0.5f - Sqrt(1.0f - x)*ret;
    return ret - 2 * negate * ret;
}



// NOTE(Momo): [-1, 1]
pure f32 ACos(f32 x) {
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
    return negate * PIf + ret;
}

// NOTE(Momo): [-1, 1]
pure f32 ATan(f32 x) {
    constexpr f32 A = 0.0776509570923569f;
    constexpr f32 B = -0.287434475393028f;
    constexpr f32 C = PIf / 4 - A - B;
    f32 xx = x * x;
    return ((A*xx + B)*xx + C)*x;
}




// NOTE(Momo): Vectors
struct Vec3f {
    union {
        f32 Arr[3];
        struct {
            f32 X;
            f32 y;
            f32 Z;
        };	
    };
    
    inline f32 operator[](usize index) const { 
        return Arr[index]; 
    } 
};

pure Vec3f Add(Vec3f lhs, Vec3f rhs) {
    return { lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z };
    
}

pure Vec3f Sub(Vec3f lhs, Vec3f rhs) {
    return { lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z };
}

pure Vec3f Mul(Vec3f lhs, f32 rhs) {
    return { lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs };
}

pure Vec3f Div(Vec3f lhs, f32 rhs) {
    Assert(IsEqual(rhs, 0.f));
    return { lhs.X / rhs, lhs.Y / rhs, lhs.Z / rhs };
}

pure Vec3f Negate(Vec3f lhs){
    return {-lhs.X, -lhs.Y, -lhs.Z};
    
}
pure bool IsEqual(Vec3f lhs, Vec3f rhs) {
    return 
        IsEqual(lhs.X, rhs.X) && 
        IsEqual(lhs.Y, rhs.Y) && 
        IsEqual(lhs.Z, rhs.Z);
}

pure f32 Dot(Vec3f lhs, Vec3f rhs) {
    return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
}

pure Vec3f operator+(Vec3f lhs, Vec3f rhs)  { 
    return Add(lhs, rhs); 
}

pure Vec3f operator-(Vec3f lhs, Vec3f rhs)  { 
    return Sub(lhs, rhs);
}

pure Vec3f operator*(Vec3f lhs, f32 rhs)  { 
    return Mul(lhs, rhs);
}

pure Vec3f operator*(float lhs, Vec3f rhs)  { 
    return Mul(rhs, lhs);
}

pure f32 operator*(Vec3f lhs, Vec3f rhs) {
    return Dot(lhs, rhs); 
}

pure Vec3f operator/(Vec3f lhs, f32 rhs)  { 
    return Div(lhs, rhs); 
}

pure Vec3f& operator+=(Vec3f& lhs, Vec3f rhs) {
    return lhs = lhs + rhs;
}

Vec3f& operator-=(Vec3f& lhs, Vec3f rhs) {
    return lhs = lhs - rhs;
}

Vec3f& operator*=(Vec3f& lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

Vec3f& operator/=(Vec3f& lhs, f32 rhs) {
    return lhs = lhs / rhs;
}

pure bool operator==(Vec3f lhs, Vec3f rhs)  { 
    return IsEqual(lhs, rhs);
}

pure bool operator!=(Vec3f lhs, Vec3f rhs) { 
    return !(lhs == rhs); 
}

pure Vec3f operator-(Vec3f lhs)  {  
    return { -lhs.X, -lhs.Y, -lhs.Z }; 
}

pure Vec3f Midpoint(Vec3f lhs, Vec3f rhs)  { 
    return (lhs + rhs)/2.f; 
}

pure f32 DistSq(Vec3f lhs, Vec3f rhs) { 
    return (rhs.X - lhs.X) * (rhs.Y - lhs.Y) * (rhs.Z - lhs.Z);
}

pure f32 LenSq(Vec3f lhs) { 
    return lhs * lhs;	
}

pure f32 Dist(Vec3f lhs, Vec3f rhs)  { 
    return Sqrt(DistSq(lhs, rhs)); 
}

pure f32 Len(Vec3f lhs)  { 
    return Sqrt(LenSq(lhs));
};

pure Vec3f Normalize(Vec3f lhs)  {
    Vec3f ret = lhs;
    f32 len = Len(lhs);
    if (IsEqual(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure f32 AngleBetween(Vec3f lhs, Vec3f rhs) {
    return ACos((lhs * rhs) / (Len(lhs) * Len(rhs)));
}

pure bool IsPerpendicular(Vec3f lhs, Vec3f rhs) 
{ 
    return IsEqual((lhs * rhs), 0.f); 
}

pure bool IsSameDir(Vec3f lhs, Vec3f rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppDir(Vec3f lhs, Vec3f rhs) { 
    return (lhs * rhs) < 0.f;
}

pure Vec3f Project(Vec3f from, Vec3f to) { 
    return (to * from) / LenSq(to) * to;
}

// NOTE(Momo): Column Major Matrices
struct Mat44f {
    f32 Arr[16];
    
    inline const f32& operator[](usize index) const { return Arr[index]; }
    inline f32& operator[](usize index) { return Arr[index];}
};

pure Mat44f CreateTranslation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        x, y, z, 1.f
    };
}


pure Mat44f CreateRotationX(f32 rad) {
    return {
        1.f,  0.f,      0.f,      0.f,
        0.f,  Cos(rad), Sin(rad), 0.f,  
        0.f, -Sin(rad), Cos(rad), 0.f,
        0.f,  0.f,      0.f,      1.f
    };
}

pure Mat44f CreateRotationY(f32 rad) {
    return {
        Cos(rad),  0.f, -Sin(rad), 0.f,
        0.f,       1.f, 0.f,       0.f,
        Sin(rad),  0.f, Cos(rad),  0.f,
        0.f,       0.f, 0.f,       1.f
    };
    
}


pure Mat44f CreateRotationZ(f32 rad) {
    return {
        Cos(rad),  Sin(rad), 0.f, 0.f,
        -Sin(rad), Cos(rad), 0.f, 0.f,
        0.f,       0.f,      1.f, 0.f,
        0.f,       0.f,      0.f, 1.f
    };
}

pure Mat44f CreateScale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}


// 
pure Mat44f CreateOrthoProjection(f32 ndcLeft, f32 ndcRight,
                                  f32 ndcBottom, f32 ndcTop,
                                  f32 ndcNear, f32 ndcFar,
                                  f32 left, f32 right, 
                                  f32 bottom, f32 top,
                                  f32 near, f32 far,
                                  bool flipZ) 
{
    return {
        (ndcRight-ndcLeft)/(right-left), 0.f, 0.f, 0.f,
        0.f, (ndcTop-ndcBottom)/(top-bottom), 0.f, 0.f,
        0.f, 0.f, (flipZ ? -1.f : 1.f) * (ndcFar-ndcNear)/(far-near), 0.f,
        -(right+left)/(right-left), 
        -(top+bottom)/(top-bottom), 
        -(far+near)/(far-near), 1.f
    };
}


pure Mat44f CreateIdentity() {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}


pure Mat44f operator*(Mat44f lhs, Mat44f rhs) {
    Mat44f res = {0};
    
    for (u8 i = 0; i < 4; i++) { 
        for (u8 j = 0; j < 4; j++) { 
            u8 index = j+i*4;
            for (u8 k = 0; k < 4; k++) 
                res[index] += lhs[j+k*4] *  rhs[i+k*4]; 
        } 
    } 
    return res;
}

#endif 