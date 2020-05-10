#ifndef __RYOJI_MATHS_BITFIELDS_H__
#define __RYOJI_MATHS_BITFIELDS_H__

#include <type_traits>
#include <cassert>
namespace ryoji::maths {
	
	/*namespace zawarudo {
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
	*/




	namespace bitfields {

		template<typename T, typename... Args>
		T bitfield(Args... args)
		{
			using namespace std;
			static_assert(conjunction_v<is_integral<T>, is_integral<Args>...>, "All types must be integral types");
			return ((1 << args) | ...);
		}

		template<typename T, typename U>
		T mask(T flag, U mask) {
			static_assert(std::is_integral_v<T>);
			return flag | mask;
		}


		template<typename T, typename U>
		T unmask(T flag, U mask) {
			static_assert(std::is_integral_v<T>);
			return flag & ~mask;
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

	using bitfields::bitfield;





}

#endif