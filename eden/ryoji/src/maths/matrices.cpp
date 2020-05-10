#include "matrices.h"

namespace ryoji::maths {
	template struct matrices::Matrix<float, 2, 2>;
	template struct matrices::Matrix<float, 3, 3>;
	template struct matrices::Matrix<float, 4, 4>;
	template struct matrices::Matrix<int, 2, 2>;
	template struct matrices::Matrix<int, 3, 3>;
	template struct matrices::Matrix<int, 4, 4>;
	template struct matrices::Matrix<double, 2, 2>;
	template struct matrices::Matrix<double, 3, 3>;
	template struct matrices::Matrix<double, 4, 4>;
}