#ifndef __RYOJI_ALLOCATORS_TRACE_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_TRACE_ALLOCATOR_H__

// Allocator that logs to std::cout whenever there's allocator and deallocation (for debugging purposes)
#include "blk.h"

namespace ryoji::allocators {  

	template<class Allocator>
	class TraceAllocator
	{
		Allocator allocator;
		size_t bytesAllocated;
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			Blk ret = allocator.allocate(size, alignment);
			if (ret) {
				bytesAllocated += size;
			}
			return ret;
		}

		void deallocate(Blk blk)
		{
			if (!blk)
				return;

			assert(owns(blk));

			bytesAllocated -= size;
			allocator.deallocate(blk);
		}

		bool owns(Blk blk) const noexcept {
			return allocator.owns(blk);
		}

		void deallocateAll() noexcept {
			allocator.deallocateAll();
		}

	};
}

#endif