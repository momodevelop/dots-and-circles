#ifndef __RYOJI_ALLOCATORS_LOCAL_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_LOCAL_ALLOCATOR_H__

// Simple allocator that reserves memory on the stack frame.
// It's basically a wrapper around an array.
// It will always return the start point of the array on 'allocation'.
// Once it's allocated, cannot allocate again until it's deallocated.
// This is useful if you do not want to reserve memory in the heap using the HeapAllocator

#include <array>



namespace ryoji::allocators {
	template<size_t Capacity>
	class LocalAllocator
	{
		// I don't think it makes sense to allow copy or move since this is supposed to be local to scope...
		LocalAllocator& operator=(const LocalAllocator&) = delete;
		LocalAllocator(const LocalAllocator&) = delete;
		LocalAllocator& operator=(LocalAllocator&&) = delete;
		LocalAllocator(LocalAllocator&&) = delete;

		std::array<char, Capacity> arr = { 0 };
		bool allocated = false;

	public:
		LocalAllocator() = default;


		void* allocate(size_t size, uint8_t alignment) noexcept
		{
			assert(size && alignment);

			if (allocated || size > Capacity)
				return {};
			else {
				allocated = true;
				return &arr.front() ;
			}
		}

		void deallocate(void* blk)   noexcept
		{
			assert(blk && owns(blk) && allocated);
			allocated = false;
		}

		bool owns(void* blk) const noexcept {
			return blk && blk == &arr.front();
		}

	private:

	};

}

#endif