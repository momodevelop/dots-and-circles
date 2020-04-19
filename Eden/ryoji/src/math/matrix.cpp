#include "matrix.h"

namespace ryoji::math {
	template struct Matrix<float, 2, 2>;
	template struct Matrix<float, 3, 3>;
	template struct Matrix<float, 4, 4>;
	template struct Matrix<int, 2, 2>;
	template struct Matrix<int, 3, 3>;
	template struct Matrix<int, 4, 4>;
	template struct Matrix<double, 2, 2>;
	template struct Matrix<double, 3, 3>;
	template struct Matrix<double, 4, 4>;
}