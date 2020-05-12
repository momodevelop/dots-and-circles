#ifndef __RYOJI_MATH_MATRICES_H__
#define __RYOJI_MATH_MATRICES_H__

//http://www.reedbeta.com/blog/on-vector-math-libraries/#operations

#include <cmath>
#include <array>
#include <type_traits>
#include <cassert>
#include "zawarudo/constants.h"
#include "utils.h"

namespace ryoji::maths {
	
	namespace matrices {
		template<typename T, size_t Row, size_t Col>
		class Matrix {
			T arr[Row][Col] = { {} };
		public:
			template<typename ... Args, typename E = std::enable_if_t<(std::is_same_v<Args, T>&& ...)>>
			Matrix(Args... args) : arr{ std::forward<Args>(args)... } {}
			Matrix() = default;

			inline const T& get(size_t row, size_t col) const noexcept { return this->arr[row][col]; }
			inline const void set(size_t row, size_t col, const T& value) noexcept { this->arr[row][col] = value; }

			Matrix<T, Row, Col>& operator+=(const Matrix<T, Row, Col>& rhs) {
				for (size_t i = 0; i < Row; ++i)
					for (size_t j = 0; j < Col; ++j)
						this->arr[i][j] += rhs.arr[i][j];
				return *this;
			}

			Matrix<T, Row, Col>& operator-=(const Matrix<T, Row, Col>& rhs) {
				for (size_t i = 0; i < Row; ++i)
					for (size_t j = 0; j < Col; ++j)
						this->arr[i][j] -= rhs.arr[i][j];
				return *this;
			}

			Matrix<T, Row, Col>& operator*=(const T& rhs) {
				for (size_t i = 0; i < Row; ++i)
					for (size_t j = 0; j < Col; ++j)
						this->arr[i][j] *= rhs;
				return *this;
			}

			Matrix<T, Row, Col>& operator/=(const T& rhs) {
				for (size_t i = 0; i < Row; ++i)
					for (size_t j = 0; j < Col; ++j)
						this->arr[i][j] /= rhs;
				return *this;
			}
		};

		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> operator+(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
			return Matrix<T, Row, Col>(lhs) += rhs;
		}

		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> operator-(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
			return Matrix<T, Row, Col>(lhs) -= rhs;
		}

		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> operator*(const Matrix<T, Row, Col>& lhs, const T& rhs) {
			return Matrix<T, Row, Col>(lhs) *= rhs;
		}

		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> operator/(const Matrix<T, Row, Col>& lhs, const T& rhs) {
			return Matrix<T, Row, Col>(lhs) /= rhs;
		}

		// Hmm: Do we need to optimize this by letting this function be a friend of matrix class?
		template<typename T, size_t LhsRow, size_t LhsCol, size_t RhsRow, size_t RhsCol>
		auto operator*(const Matrix<T, LhsRow, LhsCol>& lhs, const Matrix<T, RhsRow, RhsCol>& rhs) {
			static_assert(LhsCol == RhsRow, "LHS column != RHS row");
			Matrix<T, LhsRow, RhsCol> ret{};
			for (size_t i = 0; i < LhsRow; ++i)
				for (size_t j = 0; j < RhsCol; ++j)
					for (size_t k = 0; k < LhsCol; ++k)
						ret.set(i,j, ret.get(i,j) + lhs.get(i, k) * rhs.get(k, j));
			return ret;
		}

		template<typename T, size_t Row, size_t Col>
		bool operator==(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
			Matrix<T, Row, Col> mat;
			for (size_t i = 0; i < Row; ++i)
				for (size_t j = 0; j < Col; ++j)
					if (lhs.get(i,j) != rhs.get(i,j))
						return false;
			return true;
		}

		template<typename T, size_t Row, size_t Col>
		bool operator!=(const Matrix<T, Row, Col>& lhs, const Matrix<T, Row, Col>& rhs) {
			return !(lhs == rhs);
		}


		template<typename T, size_t Row, size_t Col>
		Matrix<T, Row, Col> identity() {
			static_assert(Row == Col, "identity matrices only works for square matrices!");
			Matrix<T, Row, Col> ret{};
			for (size_t i = 0; i < Row; ++i)
				ret.set(i, i, 1);
			return ret;
		}
	}
	
	using matrices::Matrix;
	using Mtx22f = matrices::Matrix<float, 2, 2>;
	using Mtx33f = matrices::Matrix<float, 3, 3>;
	using Mtx44f = matrices::Matrix<float, 4, 4>;
	using Mtx22i = matrices::Matrix<int, 2, 2>;
	using Mtx33i = matrices::Matrix<int, 3, 3>;
	using Mtx44i = matrices::Matrix<int, 4, 4>;
	using Mtx22d = matrices::Matrix<double, 2, 2>;
	using Mtx33d = matrices::Matrix<double, 3, 3>;
	using Mtx44d = matrices::Matrix<double, 4, 4>;

	extern template struct matrices::Matrix<float, 2, 2>;
	extern template struct matrices::Matrix<float, 3, 3>;
	extern template struct matrices::Matrix<float, 4, 4>;
	extern template struct matrices::Matrix<int, 2, 2>;
	extern template struct matrices::Matrix<int, 3, 3>;
	extern template struct matrices::Matrix<int, 4, 4>;
	extern template struct matrices::Matrix<double, 2, 2>;
	extern template struct matrices::Matrix<double, 3, 3>;
	extern template struct matrices::Matrix<double, 4, 4>;
	
}




#endif
