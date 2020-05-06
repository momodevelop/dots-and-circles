#ifndef __RYOJI_ALLOCATORS_SEGREGATOR_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_SEGREGATOR_ALLOCATOR_H_

#include <cassert>
#include "blk.h"


namespace ryoji::allocators {
	template <size_t Threshhold, class SmallAllocator, class BigAllocator>
	class SegregatorAllocator 
	{
	public:
		SmallAllocator smallAllocator;
		BigAllocator bigAllocator;

		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			if (size > Threshhold) {
				return bigAllocator.allocate(size, alignment);
			}
			else {
				return smallAllocator.allocate(size, alignment);
			}
		}

		void deallocate(Blk blk) 
		{
			assert(blk);

			if (smallAllocator.owns(blk))
				smallAllocator.deallocate(blk);
			else if (bigAllocator.owns(blk))
				bigAllocator.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const Blk& blk) const noexcept {
			return smallAllocator.owns(blk) || bigAllocator.owns(blk);
		}

		void reset() {
			smallAllocator.reset();
			bigAllocator.reset();
		}



	};
}
#endif


