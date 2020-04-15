#ifndef __RYOJI_MATH_VECTOR_H__
#define __RYOJI_MATH_VECTOR_H__

//http://www.reedbeta.com/blog/on-vector-math-libraries/#operations

#include <cmath>
#include <array>
#include <type_traits>
#include <cassert>
#include "detail/constants.h"
#include "utils.h"

#define DEFINE_SUBSCRIPT_OPS_AND_CONSTRUCTOR \
inline T& operator[](size_t index) { return value[index]; }\
inline const T& operator[](size_t index) const { return value[index]; }\

namespace ryoji::math {
	
	template<typename T = float, size_t N = 2>
	struct Vector {
		T value[N] = {};
		DEFINE_SUBSCRIPT_OPS_AND_CONSTRUCTOR
	};

	template<typename T, size_t N>
	Vector<T,N>& operator+=(Vector<T,N>& dis, const Vector<T,N>& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] += rhs[i];
		return dis;
	}

	template<typename T, size_t N>
	Vector<T,N>& operator-=(Vector<T,N>& dis, const Vector<T, N>& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] -= rhs[i];
		return dis;
	}

	template<typename T, size_t N>
	Vector<T,N>& operator*=(Vector<T,N>& dis, const T& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] *= rhs;
		return dis;
	}

	template<typename T, size_t N>
	Vector<T,N>& operator/=(Vector<T,N>& dis, const T& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] /= rhs;
		return dis;
	}

	template<typename T, size_t N>
	Vector<T,N>& operator*=(Vector<T,N>& dis, const Vector<T,N>& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] *= rhs[i];
		return dis;
	}

	template<typename T, size_t N>
	Vector<T,N>& operator/=(Vector<T,N>& dis, const Vector<T,N>& rhs) noexcept {
		for (size_t i = 0; i < N; ++i)
			dis[i] /= rhs[i];
		return dis;
	}


	template<typename T, size_t N>
	Vector<T, N> operator+(const Vector<T,N>& lhs, const Vector<T, N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] + rhs[i];
		return temp;
	}

	template<typename T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] - rhs[i];
		return temp;
	}

	template<typename T, size_t N>
	Vector<T,N> operator*(const Vector<T,N>& lhs, const T& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] * rhs;
		return temp;
	}

	template<typename T, size_t N>
	Vector<T, N> operator/(const Vector<T, N>& lhs, const T& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] / rhs;
		return temp;
	}

	
	template<typename T, size_t N>
	Vector<T, N> operator%(const Vector<T, N>& lhs, const T& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] % rhs;
		return temp;
	}

	template<typename T, size_t N>
	Vector<T,N> operator*(const T& lhs, const Vector<T,N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs * rhs[i];
		return temp;
	}


	template<typename T, size_t N>
	Vector<T,N> operator*(const Vector<T,N>& lhs, const Vector<T,N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] * rhs[i];
		return temp;
	}

	template<typename T, size_t N>
	Vector<T,N> operator/(const Vector<T,N>& lhs, const Vector<T,N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] / rhs[i];
		return temp;
	}

	template<typename T, size_t N>
	Vector<T, N> operator%(const Vector<T, N>& lhs, const Vector<T,N>& rhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = lhs[i] % rhs[i];
		return temp;
	}

	template<typename T, size_t N>
	bool operator==(const Vector<T,N>& lhs, const Vector<T, N>& rhs) noexcept
	{
		for (size_t i = 0; i < N; ++i) {
			if constexpr(std::is_floating_point_v<T>) {
				if (!utils::isFloatEqualEnough(lhs[i], rhs[i]))
					return false;
			} 
			else {
				if (lhs[i] != rhs[i])
					return false;
			}
		}
		return true;
	}

	template<typename T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& lhs) noexcept
	{
		Vector<T, N> temp;
		for (size_t i = 0; i < N; ++i)
			temp[i] = -lhs[i];
		return temp;
	}


	template<typename T, size_t N>
	bool operator!=(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	namespace vector {
		template<typename T, size_t N>
		T dot(const Vector<T, N>& lhs, const Vector<T,N>& rhs) noexcept
		{
			T ret{};
			for (size_t i = 0; i < N; ++i) {
				ret += lhs[i] * rhs[i];
			}
			return ret;
		}
		
		template<typename T, size_t N>
		Vector<T,N> midpoint(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept {
			Vector<T, N> temp;
			for (size_t i = 0; i < N; ++i)
				temp[i] = (lhs[i] + rhs[i]) / 2;
			return temp;
		}

		template<typename T, size_t N>
		T distanceSq(const Vector<T,N> & lhs, const Vector<T,N> & rhs) noexcept
		{
			T ret{};
			for (size_t i = 0; i < N; ++i) {
				ret += (rhs[i] - lhs[i]) * (rhs[i] - lhs[i]);
			}
			return ret;
		}
		


		template<typename T, size_t N>
		T lengthSq(const Vector<T, N>& lhs) noexcept
		{
			T ret{};
			for (size_t i = 0; i < N; ++i) {
				ret += lhs[i] * lhs[i];
			}
			return ret;
		}

		template<typename T, size_t N, typename TRet = T>
		T distance(const Vector<T, N> & lhs, const Vector<T, N> & rhs) noexcept
		{
			return static_cast<TRet>(sqrt(distanceSq(lhs, rhs)));
		}

		template<typename T, size_t N, typename TRet = T>
		TRet length(const Vector<T, N>& lhs) noexcept
		{
			return static_cast<TRet>(sqrt(lengthSq(lhs)));
		}

		template<typename T, size_t N>
		Vector<T,N> normalize(const Vector<T,N>& lhs) noexcept 
		{
			static_assert(std::is_floating_point_v<T>, "T must be a floating point value");
			Vector<T,N> ret = lhs;
			T len = length(lhs);
			
			if (utils::isFloatEqualEnough(len, detail::_1p0<T>))
				return ret;

			for(size_t i = 0; i < N; ++i) {
				ret[i] /= len;
			}
			return ret;
		}

		template<typename T, size_t N, typename TRet = T>
		TRet angleBetween(const Vector<T,N>& lhs, const Vector<T,N>& rhs ) {
			return static_cast<TRet>(acos(dot(lhs, rhs)/(length(lhs) * length(rhs))));
		}

		template<typename T, size_t N>
		bool isPerpendicular(const Vector<T,N>& lhs, const Vector<T,N>& rhs) {
			return utils::isFloatEqualEnough(dot(lhs, rhs), detail::zero<T>);
		}


		template<typename T, size_t N>
		bool isSimilarDirection(const Vector<T,N>& lhs, const Vector<T,N>& rhs) {
			return dot(lhs, rhs) > detail::zero<T>;
		}
	
		template<typename T, size_t N>
		bool isOppositeDirection(const Vector<T,N>& lhs, const Vector<T,N>& rhs) {
			return dot(lhs, rhs) < detail::zero<T>;
		}

		template<typename T, size_t N>
		Vector<T,N> project(const Vector<T,N>& from, const Vector<T,N>& to) {
			return (dot(to, from)/lengthSq(to)) * to;
		}

		template<typename TTo, typename TFrom, size_t N>
		Vector<TTo, N> convert(const Vector<TFrom, N>& from) {
			Vector<TTo,N> ret{};
			for (size_t i = 0; i < N; ++i) {
				ret[i] = static_cast<TTo>(from[i]);
			}
			return ret;
		}
		
		namespace normalized {

			// Optimization for vectors that are already normalized
			template<typename T, size_t N>
			T angleBetween(const Vector<T,N>& lhs, const Vector<T,N>& rhs ) {
				static_assert(std::is_floating_point_v<T>, "T must be floating point type");
				assert(utils::isFloatEqualEnough(lengthSq(lhs), detail::_1p0<T>));
				assert(utils::isFloatEqualEnough(lengthSq(rhs), detail::_1p0<T>));
				return acos(dot(lhs, rhs));
			}
		}

	}

	template <typename T> struct Vector<T,2> { union { std::array<T, 2> value; struct { T x, y; }; }; DEFINE_SUBSCRIPT_OPS_AND_CONSTRUCTOR };
	template <typename T> struct Vector<T,3> { union { std::array<T, 3> value; struct { T x, y, z; }; }; DEFINE_SUBSCRIPT_OPS_AND_CONSTRUCTOR };
	template <typename T> struct Vector<T,4> { union { std::array<T, 4> value; struct { T x, y, z, w; }; }; DEFINE_SUBSCRIPT_OPS_AND_CONSTRUCTOR };

	// Common vector typedefs
	using Vec2f = Vector<float,2>;
	using Vec3f = Vector<float,3>;
	using Vec4f = Vector<float,4>;

	using Vec2i = Vector<int, 2>;
	using Vec3i = Vector<int, 3>;
	using Vec4i = Vector<int, 4>;

	using Vec2d = Vector<double, 2>;
	using Vec3d = Vector<double, 3>;
	using Vec4d = Vector<double, 4>;
}

#undef CREATE_SUBSCRIPT_OPERATORS
#endif
