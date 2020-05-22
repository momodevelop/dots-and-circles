#ifndef __RYOJI_MATH_CONSTANTS_H__
#define __RYOJI_MATH_CONSTANTS_H__




namespace ryoji::maths {

	
	template<typename T> constexpr static auto kZero = (T)0;
	template<typename T> constexpr static auto kOne = (T)1;
	template<typename T> constexpr static auto kTwo = (T)2;

	template<typename T> constexpr static auto kPi = (T)3.14159265358979323846;

	template<typename T> constexpr static auto kFullCircle = (T)360;
	template<typename T> constexpr static auto kHalfCircle = kFullCircle<T> / 2;
	template<typename T> constexpr static auto kQuarterCircle = kFullCircle<T> / 4;
	template<typename T> constexpr static auto kEighthCircle = kFullCircle<T> / 8;
}


#endif