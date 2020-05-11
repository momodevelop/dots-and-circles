#ifndef __RYOJI_MATH_VECTORS_H__
#define __RYOJI_MATH_VECTORS_H__

//http://www.reedbeta.com/blog/on-vector-math-libraries/#operations

#include <cmath>
#include <array>
#include <type_traits>
#include <cassert>
#include "constants.h"
#include "utils.h"



namespace ryoji::maths {
	namespace vectors {

		template<typename T = float, size_t N = 2>
		class Vector {
			std::array<T, N> arr{ 0 };
		public:
			template<typename ... Args, typename E = std::enable_if_t<(std::is_same_v<Args, T>&& ...)>>
			Vector(Args... args) : arr{ std::forward<Args>(args)... } {}
			Vector() = default;

			inline const T& get(size_t index) const noexcept { return this->arr[index]; } 
			inline const void set(size_t index, const T& value) noexcept { this->arr[index] = value; }

			Vector<T, N>& operator+=(const Vector<T, N>& rhs) noexcept {
				for (size_t i = 0; i < N; ++i)
					this->arr[i] += rhs.arr[i];
				return *this;
			}
			Vector<T, N>& operator-=(const Vector<T, N>& rhs) noexcept {
				for (size_t i = 0; i < N; ++i)
					this->arr[i] -= rhs.arr[i];
				return *this;
			}
			Vector<T, N>& operator*=(const Vector<T, N>& rhs) noexcept
			{
				for (size_t i = 0; i < N; ++i)
					this->arr[i] *= rhs.arr[i];
				return *this;
			}
			Vector<T, N>& operator/=(const Vector<T, N>& rhs) noexcept {
				for (size_t i = 0; i < N; ++i)
					this->arr[i] /= rhs.arr[i];
				return *this;
			}


			Vector<T, N>& operator*=(const T& rhs) noexcept {
				for (size_t i = 0; i < N; ++i)
					this->arr[i] *= rhs;
				return *this;
			}
			Vector<T, N>& operator/=(const T& rhs) noexcept {
				for (size_t i = 0; i < N; ++i)
					this->arr[i] /= rhs;
				return *this;
			}

		};
		
		template<typename T, size_t N>
		Vector<T, N> operator+(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return Vector<T, N>(lhs) += rhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator-(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return Vector<T, N>(lhs) -= rhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator*(const Vector<T, N>& lhs, const T& rhs) noexcept
		{
			return Vector<T, N>(lhs) *= rhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator*(const T& lhs, const Vector<T, N>& rhs) noexcept
		{
			return Vector<T, N>(rhs) *= lhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator*(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return Vector<T, N>(lhs) *= rhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator/(const Vector<T, N>& lhs, const T& rhs) noexcept
		{
			return Vector<T, N>(lhs) /= rhs;
		}

		template<typename T, size_t N>
		Vector<T, N> operator/(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return Vector<T, N>(lhs) /= rhs;
		}


		template<typename T, size_t N>
		bool operator==(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			for (size_t i = 0; i < N; ++i) {
				if constexpr (std::is_floating_point_v<T>) {
					if (!isFloatEqualEnough(lhs.get(i), rhs.get(i)))
						return false;
				}
				else {
					if (lhs.get(i) != rhs.get(i))
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
				temp.set(i, -lhs.get(i));
			return temp;
		}


		template<typename T, size_t N>
		bool operator!=(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		template<typename T, size_t N>
		T dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			T ret{};
			for (size_t i = 0; i < N; ++i) 
				ret += lhs.get(i) * rhs.get(i);
			return ret;
		}

		template<typename T, size_t N>
		Vector<T, N> midpoint(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept {
			return (lhs + rhs)/ kTwo<T>;
		}

		template<typename T, size_t N>
		T distanceSq(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			T ret{};
			for (size_t i = 0; i < N; ++i) 
				ret += (rhs.get(i) - lhs.get(i)) * (rhs.get(i) - lhs.get(i));
			return ret;
		}



		template<typename T, size_t N>
		T lengthSq(const Vector<T, N>& lhs) noexcept
		{
			return dot(lhs, lhs);
		}

		template<typename T, size_t N, typename TRet = T>
		T distance(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
		{
			return static_cast<TRet>(sqrt(distanceSq(lhs, rhs)));
		}

		template<typename T, size_t N, typename TRet = T>
		TRet length(const Vector<T, N>& lhs) noexcept
		{
			return static_cast<TRet>(sqrt(lengthSq(lhs)));
		}

		template<typename T, size_t N>
		Vector<T, N> normalize(const Vector<T, N>& lhs) noexcept
		{
			static_assert(std::is_floating_point_v<T>, "T must be a floating point value");
			Vector<T, N> ret = lhs;
			T len = length(lhs);

			if (isFloatEqualEnough(len, kOne<T>))
				return ret;

			ret /= len;
			return ret;
		}

		template<typename T, size_t N, typename TRet = T>
		TRet angleBetween(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
			return static_cast<TRet>(acos(dot(lhs, rhs) / (length(lhs) * length(rhs))));
		}

		template<typename T, size_t N>
		bool isPerpendicular(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
			return isFloatEqualEnough(dot(lhs, rhs), kZero<T>);
		}


		template<typename T, size_t N>
		bool isSimilarDirection(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
			return dot(lhs, rhs) > kZero<T>;
		}

		template<typename T, size_t N>
		bool isOppositeDirection(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
			return dot(lhs, rhs) < kZero<T>;
		}

		template<typename T, size_t N>
		Vector<T, N> project(const Vector<T, N>& from, const Vector<T, N>& to) {
			return (dot(to, from) / lengthSq(to)) * to;
		}

		template<typename TTo, typename TFrom, size_t N>
		Vector<TTo, N> convert(const Vector<TFrom, N>& from) {
			Vector<TTo, N> ret{};
			for (size_t i = 0; i < N; ++i) {
				ret.set(i, static_cast<TTo>(from.get(i)));
			}
			return ret;
		}

		namespace normalized {

			// Optimization for vectors that are already normalized
			template<typename T, size_t N>
			T angleBetween(const Vector<T, N>& lhs, const Vector<T, N>& rhs) {
				static_assert(std::is_floating_point_v<T>, "T must be floating point type");
				assert(isFloatEqualEnough(lengthSq(lhs), kOne<T>));
				assert(isFloatEqualEnough(lengthSq(rhs), kOne<T>));
				return acos(dot(lhs, rhs));
			}
		}


	}


	// Common vector typedefs
	using vectors::Vector;
	using Vec2f = Vector<float,2>;
	using Vec3f = Vector<float,3>;
	using Vec4f = Vector<float,4>;
	using Vec2i = Vector<int, 2>;
	using Vec3i = Vector<int, 3>;
	using Vec4i = Vector<int, 4>;
	using Vec2d = Vector<double, 2>;
	using Vec3d = Vector<double, 3>;
	using Vec4d = Vector<double, 4>;
	
	// To be explicitly instantiated in lib file
	extern template struct Vector<float,2>;
	extern template struct Vector<float,3>;
	extern template struct Vector<float,4>;
	extern template struct Vector<int,2>;
	extern template struct Vector<int,3>;
	extern template struct Vector<int,4>;
	extern template struct Vector<double,2>;
	extern template struct Vector<double,3>;
	extern template struct Vector<double,4>;
}

#undef CREATE_SUBSCRIPT_OPERATORS
#endif
