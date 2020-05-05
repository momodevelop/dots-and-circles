#ifndef __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__

// If the Primary allocator fails, the Fallback allocator will allocate.
#include <cassert>
#include "blk.h"


namespace ryoji::allocators {
	template <class Primary, class Fallback>
	class FallbackAllocator
	{
	public:
		Primary primaryAllocator = {};
		Fallback fallbackAllocator = {};

	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			Blk blk = primaryAllocator.allocate(size, alignment);
			if (!blk) {
				return fallbackAllocator.allocate(size, alignment);
			}
	
			return blk;
		}

		void deallocate(Blk blk)
		{
			assert(blk);

			if (primary.owns(blk))
				primaryAllocator.deallocate(blk);
			else if (fallback.owns(blk))
				fallbackAllocator.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const Blk& blk) {
			return primaryAllocator.owns(blk) || fallbackAllocator.owns(blk);
		}

		void reset() {
			primaryAllocator.reset();
			fallbackAllocator.reset();
		}



	};
}


#endif