#ifndef __RYOJI_MATH_UTILS__
#define __RYOJI_MATH_UTILS__

static constexpr f32 PI_f32 = 3.14159265358979323846264338327950288f;

pure bool IsFloatEq(f32 lhs, f32 rhs) {
    return fabs(lhs - rhs) <= std::numeric_limits<f32>::epsilon();
}

// Degrees and Radians
pure f32 DegToRad(f32 degrees) {
    return degrees * PI_f32 / 180.f;
}

pure f32 RadToDeg(f32 radians) {
    return radians * 180.f / PI_f32;
}


#endif 