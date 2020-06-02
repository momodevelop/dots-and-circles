// Includes
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using usize = size_t;
using uptr = uintptr_t;

static constexpr f32 PI_f32 = 3.14159265358979323846264338327950288f;


#define pure static inline
#define Abs(x) ((x < 0) ? -x : x)
#define Max(x, y) ((x > y) x : y)
#define Min(x, y) ((x < y) x : y)


#include <stdio.h>
using namespace std;

#include <math.h>

// NOTE(Momo): [-PI,PI]
pure f32 Sin(f32 x) {
    constexpr float B = 4.0f / PI_f32;
    constexpr float C = -4.0f / (PI_f32 * PI_f32);
    constexpr float P = 0.225f;
    
    float y = B * x + C * x * Abs(x);
    return P * (y * Abs(y) - y) + y;
}

// NOTE(Momo): [-PI, PI]
pure f32 Cos(f32 x) {
    constexpr float B = 4.0f / PI_f32;
    constexpr float C = -4.0f / (PI_f32 * PI_f32);
    constexpr float P = 0.225f;
    
    x = (x > 0) ? -x : x;
    x += PI_f32/2;
    
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
f32 ASin(f32 x) {
    f32 negate = f32(x < 0);
    x = Abs(x);
    f32 ret = -0.0187293;
    ret *= x;
    ret += 0.0742610;
    ret *= x;
    ret -= 0.2121144;
    ret *= x;
    ret += 1.5707288;
    ret = PI_f32 *0.5 - Sqrt(1.0f - x)*ret;
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
    return negate * PI_f32 + ret;
}

// NOTE(Momo): [-1, 1]
pure f32 ATan(f32 x) {
    constexpr f32 A = 0.0776509570923569f;
    constexpr f32 B = -0.287434475393028f;
    constexpr f32 C = PI_f32 / 4 - A - B;
    f32 xx = x * x;
    return ((A*xx + B)*xx + C)*x;
}


int main() {
    printf("%f\nvs\n%f\n\n", Cos(1.57), cos(1.57));
    printf("%f\nvs\n%f\n\n", Sin(1.141), sin(1.141));
    printf("%f\nvs\n%f\n\n", Tan(1.141), tan(1.141));
    printf("%f\nvs\n%f\n\n", ASin(2), asin(2));
    printf("%f\nvs\n%f\n\n", ACos(2), acos(2));
    printf("%f\nvs\n%f\n\n", ATan(.41), atan(0.41));
    printf("%f\nvs\n%f\n\n", Sqrt(9), sqrt(9));
}


