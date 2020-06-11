#ifndef __RYOJI_MATH__
#define __RYOJI_MATH__

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

pure v3f Add(v3f lhs, v3f rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    
}

pure v3f Sub(v3f lhs, v3f rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

pure v3f Mul(v3f lhs, f32 rhs) {
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

pure v3f Div(v3f lhs, f32 rhs) {
    Assert(IsEqual(rhs, 0.f));
    return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

pure v3f Negate(v3f lhs){
    return {-lhs.x, -lhs.y, -lhs.z};
    
}
pure bool IsEqual(v3f lhs, v3f rhs) {
    return 
        IsEqual(lhs.x, rhs.x) && 
        IsEqual(lhs.y, rhs.y) && 
        IsEqual(lhs.z, rhs.z);
}

pure f32 Dot(v3f lhs, v3f rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

pure v3f operator+(v3f lhs, v3f rhs)  { 
    return Add(lhs, rhs); 
}

pure v3f operator-(v3f lhs, v3f rhs)  { 
    return Sub(lhs, rhs);
}

pure v3f operator*(v3f lhs, f32 rhs)  { 
    return Mul(lhs, rhs);
}

pure v3f operator*(float lhs, v3f rhs)  { 
    return Mul(rhs, lhs);
}

pure f32 operator*(v3f lhs, v3f rhs) {
    return Dot(lhs, rhs); 
}

pure v3f operator/(v3f lhs, f32 rhs)  { 
    return Div(lhs, rhs); 
}

pure v3f& operator+=(v3f& lhs, v3f rhs) {
    return lhs = lhs + rhs;
}

v3f& operator-=(v3f& lhs, v3f rhs) {
    return lhs = lhs - rhs;
}

v3f& operator*=(v3f& lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

v3f& operator/=(v3f& lhs, f32 rhs) {
    return lhs = lhs / rhs;
}

pure bool operator==(v3f lhs, v3f rhs)  { 
    return IsEqual(lhs, rhs);
}

pure bool operator!=(v3f lhs, v3f rhs) { 
    return !(lhs == rhs); 
}

pure v3f operator-(v3f lhs)  {  
    return { -lhs.x, -lhs.y, -lhs.z }; 
}

pure v3f Midpoint(v3f lhs, v3f rhs)  { 
    return (lhs + rhs)/2.f; 
}

pure f32 DistSq(v3f lhs, v3f rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
}

pure f32 LenSq(v3f lhs) { 
    return lhs * lhs;	
}

pure f32 Dist(v3f lhs, v3f rhs)  { 
    return Sqrt(DistSq(lhs, rhs)); 
}

pure f32 Len(v3f lhs)  { 
    return Sqrt(LenSq(lhs));
};

pure v3f Normalize(v3f lhs)  {
    v3f ret = lhs;
    f32 len = Len(lhs);
    if (IsEqual(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure f32 AngleBetween(v3f lhs, v3f rhs) {
    return ACos((lhs * rhs) / (Len(lhs) * Len(rhs)));
}

pure bool IsPerpendicular(v3f lhs, v3f rhs) 
{ 
    return IsEqual((lhs * rhs), 0.f); 
}

pure bool IsSameDir(v3f lhs, v3f rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppDir(v3f lhs, v3f rhs) { 
    return (lhs * rhs) < 0.f;
}

pure v3f Project(v3f from, v3f to) { 
    return (to * from) / LenSq(to) * to;
}

// NOTE(Momo): Column Major Matrices
struct m4f {
    f32 arr[16];
};

pure m4f Translation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        x, y, z, 1.f
    };
}


pure m4f RotationX(f32 rad) {
    return {
        1.f,  0.f,      0.f,      0.f,
        0.f,  Cos(rad), Sin(rad), 0.f,  
        0.f, -Sin(rad), Cos(rad), 0.f,
        0.f,  0.f,      0.f,      1.f
    };
}

pure m4f RotationY(f32 rad) {
    return {
        Cos(rad),  0.f, -Sin(rad), 0.f,
        0.f,       1.f, 0.f,       0.f,
        Sin(rad),  0.f, Cos(rad),  0.f,
        0.f,       0.f, 0.f,       1.f
    };
    
}


pure m4f RotationZ(f32 rad) {
    return {
        Cos(rad),  Sin(rad), 0.f, 0.f,
        -Sin(rad), Cos(rad), 0.f, 0.f,
        0.f,       0.f,      1.f, 0.f,
        0.f,       0.f,      0.f, 1.f
    };
}

pure m4f Scale(f32 x, f32 y, f32 z) {
    return {
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}


pure m4f OrthoProjection(f32 wmin, f32 wmax, 
                         f32 hmin, f32 hmax,
                         f32 dmin, f32 dmax) {
    
    
    
    return {
        2.f/(wmax-wmin), 0.f, 0.f, 0.f,
        0.f, 2.f/(hmax-hmin), 0.f, 0.f,
        0.f, 0.f, 2.f/(dmax-dmin), 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}


#endif 