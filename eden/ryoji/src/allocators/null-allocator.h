#ifndef __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_STACKFRAME_ALLOCATOR_H__




namespace ryoji::allocators {
	class NullAllocator
	{
	public:
		void* allocate(size_t size, uint8_t alignment) noexcept {
			return {};
		}

		void deallocate(void* blk) noexcept {}

		bool owns(void* blk) const noexcept {
			return false;
		}

	};

}

#endif