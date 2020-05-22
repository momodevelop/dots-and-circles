#ifndef __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_FALLBACK_ALLOCATOR_H__

// If the Primary allocator fails, the Fallback allocator will allocate.
#include <cassert>



namespace ryoji::allocators {
	template <class Primary, class Fallback>
	class FallbackAllocator
	{
	private:
		Primary primaryAllocator = {};
		Fallback fallbackAllocator = {};

	public:
		void* allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			void* blk = primaryAllocator.allocate(size, alignment);
			if (!blk) {
				return fallbackAllocator.allocate(size, alignment);
			}
	
			return blk;
		}

		void deallocate(void* blk)
		{
			assert(blk);

			if (primary.owns(blk))
				primaryAllocator.deallocate(blk);
			else if (fallback.owns(blk))
				fallbackAllocator.deallocate(blk);
			else
				assert(false);
		}

		bool owns(const void*& blk) {
			return primaryAllocator.owns(blk) || fallbackAllocator.owns(blk);
		}

		Primary& getPrimaryAllocator() { return primaryAllocator; }
		Fallback& getFallbackAllocator() { return fallbackAllocator; }


	};
}


#endif