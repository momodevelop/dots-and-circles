#ifndef __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__

// If the Primary allocator fails, the Fallback allocator will allocate.
#include <cassert>
#include "blk.h"


namespace ryoji::allocators {
	template <class Primary, class Fallback>
	class FallbackAllocator
	{
		Primary primary = {};
		Fallback fallback = {};
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			Blk blk = primary.allocate(size, alignment);
			if (!blk) {
				return fallback.allocate(size, alignment);
			}
	
			return blk;
		}

		void deallocate(Blk blk) noexcept  // Use pointer if pointer is not a value_type*
		{
			if (!blk)
				return;

			if (primary.owns(blk))
				primary.deallocate(blk);
			else if (fallback.owns(blk))
				fallback.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const Blk& blk) const {
			return primary.owns(blk) || fallback.owns(blk);
		}



	};
}


#endif