#ifndef __RYOJI_MATH_UTILS__
#define __RYOJI_MATH_UTILS__

pure bool IsFloatEq(f32 lhs, f32 rhs) {
    return fabs(lhs - rhs) <= std::numeric_limits<f32>::epsilon();
}


pure f32 DegToRad(f32 degrees) {
    return degrees * PIf / 180.f;
}

pure f32 RadToDeg(f32 radians) {
    return radians * 180.f / PIf;
}

#endif 