#ifndef __RYOJI_MATH_UTILS_H__
#define __RYOJI_MATH_UTILS_H__

#include <limits>
#include <type_traits>
#include <cmath>

#include "detail/constants.h"

namespace ryoji::math {
    namespace utils {
        
        template<typename T>
        constexpr static auto pi = detail::pi<T>;

        template<typename T>
        bool isFloatEqualEnough(T lhs, T rhs) {
            static_assert(std::is_floating_point_v<T>);
            return fabs(lhs - rhs) <= std::numeric_limits<float>::epsilon();
        }

        template<typename T>
        T degreesToRadians(T degrees) {
            static_assert(std::is_floating_point_v<T>, "T must be a floating point type");
            return degrees * pi<T> / detail::_180p0<T>;
        }

        template<typename T>
        T radiansToDegrees(T radians) {
            static_assert(std::is_floating_point_v<T>, "T must be a floating point type");
            return radians * detail::_180p0<T> / pi<T>;
        };
    }
        
}



#endif 