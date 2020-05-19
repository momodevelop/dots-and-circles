#ifndef __RYOJI_ALLOCATORS_HEAP_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_HEAP_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.
#include <cassert>



namespace ryoji::allocators {
	class HeapAllocator
	{
	public:
		void* allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);
			return { ::operator new(size) };
		}

		void deallocate(void* blk)  
		{
			::operator delete(blk);
		}

		bool owns(void* blk) const noexcept {
			return true;
		}
	};

}

#endif