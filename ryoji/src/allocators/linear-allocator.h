#ifndef __RYOJI_ALLOCATORS_LINEAR_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_LINEAR_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include <functional>


#include "zawarudo/pointer.h"
#include "local-allocator.h"
#include "heap-allocator.h"

namespace ryoji::allocators {

	template<size_t Capacity, class Allocator>
	class LinearAllocator
	{
		static_assert(Capacity != 0);
		struct null_deleter { template<typename T> void operator()(T*){} };
		template<typename T> using move_ptr = std::unique_ptr<T, null_deleter>;

		LinearAllocator& operator=(const LinearAllocator&) = delete;
		LinearAllocator(const LinearAllocator&) = delete;
		LinearAllocator& operator=(LinearAllocator&&) = delete;

		void* memoryBlk{};
		move_ptr<char> start{ nullptr };
		move_ptr<char> current{ nullptr };

		Allocator allocator;
	public:
		LinearAllocator() {
			memoryBlk = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memoryBlk);
			start.reset(reinterpret_cast<char*>(memoryBlk));
			current.reset(reinterpret_cast<char*>(memoryBlk));
		}

		~LinearAllocator() {
			allocator.deallocate(memoryBlk);
		}

		LinearAllocator(LinearAllocator&&) = default;

		void* allocate(size_t size, uint8_t alignment) noexcept
		{
			assert(size && alignment);

			uint8_t adjustment = zawarudo::pointer::getAlignForwardDiff(current.get(), alignment);

			// if not enough space, return nullptr
			if (current.get() + adjustment + size > start.get() + Capacity) {
				return {};
			}

			// otherwise, get the aligned address
			char* alignedAddress = current.get() + adjustment;
			current.reset(alignedAddress + size);

			return alignedAddress;
		}

		void deallocate(void* blk) noexcept
		{
			// does nothing
		}

		bool owns(void* blk) const noexcept {
			return blk && blk >= start.get() && blk < start.get() + Capacity;
		}

		Allocator& getAllocator() {
			return allocator;
		}

	};


	template<size_t Capacity>
	using LocalLinearAllocator = LinearAllocator<Capacity, LocalAllocator<Capacity>>;

	template<size_t Capacity>
	using HeapLinearAllocator = LinearAllocator<Capacity, HeapAllocator>;
}

#endif 
