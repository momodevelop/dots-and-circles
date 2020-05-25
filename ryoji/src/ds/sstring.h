#ifndef __RYOJI_DS_SSTRING_H__
#define __RYOJI_DS_SSTRING_H__

#include <assert.h>

namespace ryoji::ds {
	// 'static string'. Keeps memory on stack, never allocates on the heap.
	namespace sstring {
		template<size_t Capacity>
		struct SString {
			char buffer[Capacity] = { 0 };
			static constexpr size_t capacity = Capacity;
			size_t length = 0;

			// Sadly, we can't overload this as non-member :(
			inline SString& operator=(const char* rhs);

			template<size_t RCapacity>
			inline SString<Capacity>& operator=(const SString<RCapacity>& rhs);

		};

		template<size_t Capacity>
		inline void clear(SString<Capacity>& lhs) {
			memset(&lhs, 0, sizeof(SString<Capacity>)); // also sets length to zero :)
		}

		// assumes const char is cstring
		template<size_t Capacity>
		SString<Capacity>& set(SString<Capacity>& lhs, const char* rhs) {
			if (rhs == nullptr) 
				return lhs;

			for (lhs.length = 0; lhs.length < Capacity - 1 && rhs[lhs.length] != 0; ++lhs.length) {
				lhs.buffer[lhs.length] = rhs[lhs.length];
			}
			lhs.buffer[lhs.length] = 0;
			return lhs;
		}

		template<size_t LCapacity, size_t RCapacity>
		inline SString<LCapacity>& set(SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return set(lhs, rhs.buffer);
		}

		// optimized for same capacity
		template<size_t Capacity>
		inline SString<Capacity>& set(SString<Capacity>& lhs, const SString<Capacity>& rhs) {
			memcpy(&lhs, &rhs, sizeof(SString<Capacity>));
			return lhs;
		}


		template<size_t Capacity>
		SString<Capacity>& concat(SString<Capacity>& lhs, const char* rhs) {
			if (rhs == nullptr) 
				return lhs;

			for (size_t i = 0; lhs.length < Capacity - 1 && rhs[i] != 0; ++lhs.length, ++i) {
				lhs.buffer[lhs.length] = rhs[i];
			}
			lhs.buffer[lhs.length] = 0;
			return lhs;
		}

		template<size_t LCapacity, size_t RCapacity>
		inline SString<LCapacity>& concat(SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			// in case they are the same address, we store the rhs.length and only copy that amount of characters.
			for (size_t i = 0, rhsLength = rhs.length; lhs.length < LCapacity - 1 && i != rhsLength; ++lhs.length, ++i) {
				lhs.buffer[lhs.length] = rhs.buffer[i];
			}
			lhs.buffer[lhs.length] = 0;
			return lhs;
		}

		template<size_t Capacity>
		inline const char* get(const SString<Capacity>& lhs) {
			return lhs.buffer;
		}

		template<size_t Capacity>
		bool isEqual(const SString<Capacity>& lhs, const char* rhs) {
			for (size_t i = 0; i < lhs.length; ++i) {
				if (lhs.buffer[i] != rhs[i])
					return false;
			}
			return true;
		}

		template<size_t LCapacity, size_t RCapacity>
		bool isEqual(const SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			if (lhs.length != rhs.length) 
				return false;

			for (size_t i = 0; i < lhs.length; ++i) {
				if (lhs.buffer[i] != rhs.buffer[i])
					return false;
			}
			return true;
		}

		template<size_t Capacity>
		inline bool isEmpty(const SString<Capacity>& lhs) {
			return lhs.length == 0;
		}


		// operator overloads //
		template<size_t Capacity>
		inline bool operator==(const SString<Capacity>& lhs, const char* rhs) {
			return isEqual(lhs, rhs);
		}

		template<size_t Capacity>
		inline bool operator!=(const SString<Capacity>& lhs, const char* rhs) {
			return !isEqual(lhs, rhs);
		}

		template<size_t LCapacity, size_t RCapacity>
		inline bool operator==(const SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return isEqual(lhs, rhs);
		}

		template<size_t LCapacity, size_t RCapacity>
		inline bool operator!=(const SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return !isEqual(lhs, rhs);
		}

		template<size_t Capacity>
		inline SString<Capacity>& SString<Capacity>::operator=(const char* rhs) {
			return set(*this, rhs);
		}

		template<size_t LCapacity, size_t RCapacity>
		inline SString<LCapacity>& operator<<=(SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return set(lhs, rhs);
		}

		template<size_t Capacity>
		inline SString<Capacity>& operator+=(SString<Capacity>& lhs, const char* rhs) {
			return concat(lhs, rhs);
		}

		template<size_t LCapacity, size_t RCapacity>
		inline SString<LCapacity>& operator+=(SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return concat(lhs, rhs);
		}

		template<size_t LCapacity>
		inline SString<LCapacity>& operator<<(SString<LCapacity>& lhs, const char* rhs) {
			return concat(lhs, rhs);
		}

		template<size_t LCapacity, size_t RCapacity>
		inline SString<LCapacity>& operator<<(SString<LCapacity>& lhs, const SString<RCapacity>& rhs) {
			return concat(lhs, rhs);
		}

		template<size_t Capacity> template<size_t RCapacity>
		inline SString<Capacity>& SString<Capacity>::operator=(const SString<RCapacity>& rhs)
		{
			return set(*this, rhs);
		}


}

	using sstring::SString;
}

#endif