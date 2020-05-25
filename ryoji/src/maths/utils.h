#ifndef __RYOJI_MATH_UTILS_H__
#define __RYOJI_MATH_UTILS_H__

#include <limits>
#include <type_traits>
#include <cmath>

#include "constants.h"

namespace ryoji::maths {

    template<typename T>
    bool isFloatEqualEnough(T lhs, T rhs) {
        static_assert(std::is_floating_point_v<T>);
        return fabs(lhs - rhs) <= std::numeric_limits<float>::epsilon();
    }

    template<typename T>
    T degreesToRadians(T degrees) {
        static_assert(std::is_floating_point_v<T>, "T must be a floating point type");
        return degrees * kPi<T> / kHalfCircle<T>;
    }

    template<typename T>
    T radiansToDegrees(T radians) {
        static_assert(std::is_floating_point_v<T>, "T must be a floating point type");
        return radians * kHalfCircle<T> / kPi<T>;
    };

}



#endif 