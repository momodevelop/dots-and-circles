#ifndef __RYOJI_MATH__
#define __RYOJI_MATH__

global constexpr f32 PIf = 3.14159265358979323846264338327950288f;
global EPSILONf = 1.19209290E-07f;

pure bool IsFloatEq(f32 lhs, f32 rhs) {
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
    f32 ret = -0.0187293;
    ret *= x;
    ret += 0.0742610;
    ret *= x;
    ret -= 0.2121144;
    ret *= x;
    ret += 1.5707288;
    ret = PIf *0.5 - Sqrt(1.0f - x)*ret;
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
    ret = ret * Sqrt(1.0-x);
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


// NOTE(Momo): Bitfields
pure u8 MaskBits(u8 flag, u8 mask) { 
    return flag | mask; 
}

pure u16 MaskBits(u16 flag, u16 mask) { 
    return flag | mask;
}

pure u32 MaskBits(u32 flag, u32 mask) { 
    return flag | mask; 
}

pure u64 MaskBits(u64 flag, u64 mask) {
    return flag | mask; 
}

pure u8 UnmaskBits(u8 flag, u8 mask) 	{ 
    return flag & ~mask; 
}

pure u16 UnmaskBits(u16 flag, u16 mask)  { 
    return flag & ~mask; 
}

pure u32 UnmaskBits(u32 flag, u32 mask)  {
    return flag & ~mask; 
}

pure u64 UnmaskBits(u64 flag, u64 mask)  {
    return flag & ~mask; 
}

pure u8 SetBits(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u16 SetBits(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u32 SetBits(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u64 SetBits(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & ~(1ULL << index)) | (val << index);
}

pure u8 GetBits(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u16 GetBits(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u32 GetBits(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u64 GetBits(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & (1UL << index)) > 0;
}

pure bool AreAnyBitsOn(u8  flag, u8 mask) { 
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u16 flag, u16 mask) { 
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u32 flag, u32 mask) {
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u64 flag, u64 mask) { 
    return (flag & mask) > 0;
}

pure bool AreAllBitsOn(u8 flag, u8 mask) {
    return flag == mask; 
}

pure bool AreAllBitsOn(u16 flag, u16 mask) { 
    return flag == mask;
}

pure bool AreAllBitsOn(u32 flag, u32 mask) { 
    return flag == mask;
}

pure bool AreAllBitsOn(u64 flag, u64 mask) { 
    return flag == mask; 
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
}


pure v3f& operator+=(v3f rhs)  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

pure v3f& operator-=(v3f rhs)  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}


pure v3f& operator*=(v3f rhs)  {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}

pure v3f& operator/=(v3f rhs)  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

pure v3f& operator*=(float rhs)  {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}

pure v3f& operator/=(float rhs)  {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
}

};


pure v3f operator+(v3f lhs, v3f rhs)  { 
    return v3f(lhs) += rhs; 
}

pure v3f operator-(v3f lhs, v3f rhs)  { 
    return v3f(lhs) -= rhs;
}

pure v3f operator*(v3f lhs, float rhs)  { 
    return v3f(lhs) *= rhs; 
}

pure v3f operator*(float lhs, v3f rhs)  { 
    return v3f(rhs) *= lhs; 
}

pure v3f operator/(v3f lhs, float rhs)  { 
    return v3f(lhs) /= rhs; 
}

//NOTE(Momo): Dot product
pure float operator*(v3f lhs, v3f rhs) { 
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; 
}


pure bool operator==(v3f lhs, v3f rhs)  { 
    return 
        IsFloatEq(lhs.x, rhs.x) && 
        IsFloatEq(lhs.y, rhs.y) && 
        IsFloatEq(lhs.z, rhs.z);
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

pure f32 DistanceSq(v3f lhs, v3f rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
}

pure f32 LengthSq(v3f lhs) { 
    return lhs * lhs;	
}

pure f32 Distance(v3f lhs, v3f rhs)  { 
    return Sqrt(DistanceSq(lhs, rhs)); 
}

pure f32 Length(v3f lhs)  { 
    return Sqrt(LengthSq(lhs));
};

pure v3f Normalize(v3f lhs)  {
    v3f ret = lhs;
    f32 len = Length(lhs);
    if (IsFloatEq(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure f32 AngleBetween(v3f lhs, v3f rhs) {
    return ACos((lhs * rhs) / (Length(lhs) * Length(rhs)));
}

pure f32 AngleBetweenNormalized(v3f lhs, v3f rhs) {
    Assert(IsFloatEqual(LengthSq(lhs), 1.f));
    Assert(IsFloatEqual(LengthSq(rhs), 1.f));
    return ACos(lhs * rhs);
}

pure bool IsPerpendicular(v3f lhs, v3f rhs) 
{ 
    return IsFloatEq((lhs * rhs), 0.f); 
}

pure bool IsSameDirection(v3f lhs, v3f rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppositeDirection(v3f lhs, v3f rhs) { 
    return (lhs * rhs) < 0.f;
}

pure v3f Project(v3f from, v3f to) { 
    return (to * from) / LengthSq(to) * to;
}

// NOTE(Momo): Column Major Matrices
struct m4f {
    f32 arr[16];
};

pure m4f MakeTranslation(f32 x, f32 y, f32 z) {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        x, y, z, 1.f
    };
}


#if 0
m4f Rotate3(f32 radians) {
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        Cos(radians), 0.f, 0.f, 1.f
    }
}
#endif

#endif 