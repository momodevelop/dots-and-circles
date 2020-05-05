#ifndef __RYOJI_ALLOCATORS_HEAP_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_HEAP_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.
#include <cassert>
#include "blk.h"


namespace ryoji::allocators {
	class HeapAllocator
	{
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);
			return { ::operator new(size),	size };
		}

		void deallocate(Blk blk)  
		{
			::operator delete(blk.ptr);
		}

		bool owns(Blk blk) const noexcept {
			return true;
		}

		void reset() {}

	};

}

#endif