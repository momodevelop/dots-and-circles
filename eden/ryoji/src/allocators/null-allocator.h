#ifndef __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__

#include "blk.h"


namespace ryoji::allocators {
	class NullAllocator
	{
	public:
		Blk allocate(size_t size, uint8_t alignment) noexcept {
			return {};
		}

		void deallocate(Blk blk) noexcept {}

		bool owns(Blk blk) const noexcept {
			return false;
		}

	};

}

#endif