#include "vector.h"

namespace ryoji::math {
    template struct Vector<float,2>;
    template struct Vector<float,3>;
    template struct Vector<float,4>;

    template struct Vector<int,2>;
    template struct Vector<int,3>;
    template struct Vector<int,4>;

    template struct Vector<double,2>;
    template struct Vector<double,3>;
    template struct Vector<double,4>;
}