#ifndef __RYOJI_MATHS_BITWISE_H__
#define __RYOJI_MATHS_BITWISE_H__

#include <type_traits>
#include <cassert>
namespace ryoji::maths::bitwise {
	
	namespace zawarudo {
		template<typename T, size_t... Indices>
		struct bitfield;

		// Normal case
		template<typename T, size_t Index, size_t... Indices>
		struct bitfield<T, Index, Indices...> {
			static_assert(std::is_integral_v<T>);
			constexpr static T value = bitfield<T, Index>::value + bitfield<T, Indices...>::value;
		};

		// Base case
		template<typename T, size_t Index>
		struct bitfield<T, Index> {
			static_assert(std::is_integral_v<T>);
			constexpr static T value = 1 << Index;
		};
	}

	template<typename T, size_t... Indices> 
	constexpr static T bitfield = zawarudo::bitfield<T, Indices...>::value;


	template<typename T, typename U>
	T mask(T flag, U mask) {
		static_assert(std::is_integral_v<T>);
		return flag |= mask;
	}


	template<typename T, typename U>
	T unmask(T flag, U mask) {
		static_assert(std::is_integral_v<T>);
		return flag &= ~mask;
	}

	template<typename T>
	T set(T flag, size_t index, bool val) {
		//https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
		assert((sizeof(T) * 8) > (index - 1)); // index cannot be more than bits in T
		return (flag & ~(1UL << index)) | (val << index);
	}

	template<typename T>
	bool get(T flag, size_t index) {
		assert((sizeof(T) * 8) > (index - 1)); // index cannot be more than bits in T
		return (flag & (1UL << index)) > 0;
	}

	template<typename T, typename U>
	bool any(T flag, U mask) {
		static_assert(std::is_integral_v<T>);
		return (flag & mask) > 0;
	}

	template<typename T, typename U>
	bool all(T flag, U mask) {
		static_assert(std::is_integral_v<T>);
		return flag == mask;
	}





}

#endif