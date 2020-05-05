#ifndef __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__

#include "blk.h"


namespace ryoji::allocators {
	class NullAllocator
	{
	public:
		Blk allocate(size_t size, uint8_t alignment) {
			return {};
		}

		void deallocate(Blk blk) {}

		bool owns(Blk blk) const noexcept {
			return false;
		}

		void reset() noexcept {}
	};

}

#endif