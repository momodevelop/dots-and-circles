#ifndef __RYOJI_ALLOCATORS_SEGREGATOR_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_SEGREGATOR_ALLOCATOR_H_

#include <cassert>



namespace ryoji::allocators {
	template <size_t Threshhold, class SmallAllocator, class BigAllocator>
	class SegregatorAllocator 
	{
		SmallAllocator smallAllocator;
		BigAllocator bigAllocator;
	public:
		void* allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			if (size > Threshhold) 
				return bigAllocator.allocate(size, alignment);
			else
				return smallAllocator.allocate(size, alignment);
		}

		void deallocate(void* blk) 
		{
			assert(blk);

			if (smallAllocator.owns(blk))
				smallAllocator.deallocate(blk);
			else if (bigAllocator.owns(blk))
				bigAllocator.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const void*& blk) const noexcept {
			return smallAllocator.owns(blk) || bigAllocator.owns(blk);
		}

		BigAllocator& getBigAllocator() {
			return bigAllocator;
		}

		SmallAllocator& getSmallAllocator() {
			return smallAllocator;
		}

	};
}
#endif


