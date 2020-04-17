#ifndef __RYOJI_ALLOCATORS_DETAIL_POINTER_H__
#define __RYOJI_ALLOCATORS_DETAIL_POINTER_H__

#include <cstdint>
#include <cstddef>

// Related articles:
// Why aligned memory? -> https://www.ibm.com/developerworks/library/pa-dalign/

namespace ryoji::allocators {
	namespace zombification {
		namespace pointer {
			// Note that all alignment operations is catered to only power of 2!
			// Reference: https://stackoverflow.com/questions/227897/how-to-alloc-aligned-memory-only-using-the-standard-library
			// 
			// Let addressToAlign = A, byteToAlign = N
			// To align A to N-byte is to align A to a byte that is 2^N.
			// e.g. To align 0011 to 4-byte -> the result should be 0100 = 4
			// e.g. To align 0110 to 4-byte -> the result should be 1000 = 8
			//
			// To align, we need to remove the log2(N) least significant bits from A.
			// This means that for 2 byte alignment, we want to remove ONLY the last bits and retain the rest.
			// 2 byte alignment -> remove least significant 1 bit.
			// 4 byte alignment -> remove least significant 2 bits.
			// 8 byte alignment -> remove least significant 3 bits.
			// e.g. To align to 2 bytes (0010), we need to do A & (1110). 
			// e.g. To align to 4 bytes (0100), we need to do A & (1100). 
			// e.g. To align to 8 bytes (1000), we need to do A & (1000).
			// and so on and so forth...
			// Thus, the 'mask' can be defined as ~(N-1) 
			// e.g.  2 bytes -> ~(0010 - 1) = ~0001 = 1110
			// e.g.  4 bytes -> ~(0100 - 1) = ~0011 = 1100
			// e.g.  8 bytes -> ~(1000 - 1) = ~0111 = 1000
			// And thus, the forumla for backward alignment is: A & ~(N-1)
			static uintptr_t getAlignBackward(uintptr_t alignee, uint8_t alignment) noexcept {
				return alignee & ~(alignment - 1);
			}
			template<typename T>
			static T* getAlignBackward(T* alignee, uint8_t alignment) noexcept {
				return reinterpret_cast<char*>(getAlignBackward(reinterpret_cast<uintptr_t>(alignee), alignment));
			}



			// The functions below return the difference in bytes between the given addresses and 
			// the alignment they need to align to (backwards)
			// If the forward alignment formula is: (A & ~(N-1))
			// Then the formula for the difference is the the original address minus the result: 
			// A - (A & ~(N-1))
			static uint8_t getAlignBackwardDiff(uintptr_t addressToAlign, uint8_t alignment) noexcept {
				return static_cast<uint8_t>(addressToAlign - getAlignBackward(addressToAlign, alignment));
			}

			template<typename T>
			static uint8_t getAlignBackwardDiff(T* addressToAlign, uint8_t alignment) noexcept {
				return static_cast<uint8_t>(addressToAlign - getAlignBackward(addressToAlign, alignment));
			}


			// To align forward, we simply add N-1 to the A, then do an align backward.
			// We don't add N to A because if A is ALREADY aligned, we want it to remain as the same value.
			// Thus the completed formular is: (A + (N-1)) & (~(N-1))
			// e.g. 4-byte alignment of 0110 -> (0110 + (0100 - 1) & (~(0100 - 1)) = (1001 & 0100) = 1000
			static uintptr_t getAlignForward(uintptr_t alignee, uint8_t alignment) noexcept {
				return (alignee + (alignment - 1)) & ~(alignment - 1);
			}

			template<typename T>
			static T* getAlignForward(T* alignee, uint8_t alignment) noexcept {
				return reinterpret_cast<T*>(getAlignForward(reinterpret_cast<uintptr_t>(alignee), alignment));
			}


			// The functions below return the difference in bytes between the given addresses and 
			// the alignment they need to align to (forward)
			// If the forward alignment formula is: (A + (N-1)) & (~(N-1))
			// Then the formula for the difference is the result minus the original address: 
			// ((A + (N-1)) & (~(N-1))) - A
			static uint8_t getAlignForwardDiff(uintptr_t addressToAlign, uint8_t alignment) noexcept {
				return static_cast<uint8_t>(getAlignForward(addressToAlign, alignment) - addressToAlign);
			}

			template<typename T>
			static uint8_t getAlignForwardDiff(T* addressToAlign, uint8_t alignment) noexcept {
				return static_cast<uint8_t>(getAlignForward(addressToAlign, alignment) - addressToAlign);
			}


			template<typename T>
			static T* add(T* lhs, uintptr_t rhs) {
				return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(lhs) + rhs);
			}

			template<typename T>
			static T* sub(T* lhs, uintptr_t rhs) {
				return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(lhs) - rhs);
			}

			static size_t roundToAlignment(size_t size, uint8_t alignment)
			{
				size_t ret = size / alignment * alignment;
				return (ret == size) ? ret : ret + alignment;
			}
		}
	}

}

#endif
