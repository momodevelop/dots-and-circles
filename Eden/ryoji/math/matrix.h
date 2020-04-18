#ifndef __RYOJI_MATH_MATRIX_H__
#define __RYOJI_MATH_MATRIX_H__

//http://www.reedbeta.com/blog/on-vector-math-libraries/#operations

#include <cmath>
#include <array>
#include <type_traits>
#include <cassert>
#include "zawarudo/constants.h"
#include "utils.h"


namespace ryoji::math {
	
	template<typename T, size_t Row, size_t Col>
	struct Matrix {
		T value[Row][Col] = {{}};
		inline decltype(auto) operator[](size_t index) { return value[index]; }
		inline decltype(auto) operator[](size_t index) const { return value[index]; }
	};

	template<typename T, size_t Row, size_t Col>
	Matrix<T, Row, Col> operator+(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
		Matrix<T, Row, Col> mat;
		for (size_t i = 0; i < Row; ++i) 
			for  (size_t j = 0; j < Col; ++j) 
				mat[i][j] = lhs[i][j] + rhs[i][j];
		return mat;
	}

	template<typename T, size_t Row, size_t Col>
	Matrix<T, Row, Col> operator-(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
		Matrix<T, Row, Col> mat;
		for (size_t i = 0; i < Row; ++i) 
			for  (size_t j = 0; j < Col; ++j) 
				mat[i][j] = lhs[i][j] - rhs[i][j];	
		return mat;
	}


	template<typename T, size_t LhsRow,  size_t LhsCol, size_t RhsRow, size_t RhsCol>
	auto operator*(const Matrix<T, LhsRow, LhsCol>& lhs, const Matrix<T, RhsRow, RhsCol>& rhs) {
		static_assert(LhsCol == RhsRow, "LHS column != RHS row");
		Matrix<T, LhsRow, RhsCol> ret {};
		for (size_t i = 0; i < LhsRow; ++i) 
			for (size_t j = 0; j < RhsCol; ++j) 
				for(size_t k = 0; k < LhsCol; ++k) 
					ret[i][j] += lhs[i][k] * rhs[k][j];
		return ret;
	}

	template<typename T, size_t Row, size_t Col>
	bool operator==(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
		Matrix<T, Row, Col> mat;
		for (size_t i = 0; i < Row; ++i) 
			for  (size_t j = 0; j < Col; ++j) 
				if( lhs[i][j] != rhs[i][j] )
					return false;
		return true;
	}

	template<typename T, size_t Row, size_t Col>
	bool operator!=(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
		return !(lhs == rhs);
	}

	namespace matrix {
		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> identity() {
			static_assert(Row == Col, "identity matrices only works for square matrices!");
			Matrix<T, Row, Col> ret {};
			for (size_t i = 0; i < Row; ++i ) 
				ret[i][i] = 1;
			return ret;
		}
	}
	// typedefs
	using Mat22f = Matrix<float, 2, 2>;
	using Mat33f = Matrix<float, 3, 3>;
	using Mat44f = Matrix<float, 4, 4>;

	using Mat22i = Matrix<int, 2, 2>;
	using Mat33i = Matrix<int, 3, 3>;
	using Mat44i = Matrix<int, 4, 4>;

	using Mat22d = Matrix<double, 2, 2>;
	using Mat33d = Matrix<double, 3, 3>;
	using Mat44d = Matrix<double, 4, 4>;
	
}




#endif
