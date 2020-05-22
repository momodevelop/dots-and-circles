#include "matrix.h"

namespace ryoji::maths {
	template struct matrix::Matrix<float, 2, 2>;
	template struct matrix::Matrix<float, 3, 3>;
	template struct matrix::Matrix<float, 4, 4>;
	template struct matrix::Matrix<int, 2, 2>;
	template struct matrix::Matrix<int, 3, 3>;
	template struct matrix::Matrix<int, 4, 4>;
	template struct matrix::Matrix<double, 2, 2>;
	template struct matrix::Matrix<double, 3, 3>;
	template struct matrix::Matrix<double, 4, 4>;
}