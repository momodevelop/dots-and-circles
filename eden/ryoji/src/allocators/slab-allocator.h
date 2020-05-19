#ifndef __RYOJI_ALLOCATORS_SLAB_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_SLAB_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>


#include "zawarudo/pointer.h"
#include "local-allocator.h"
#include "heap-allocator.h"

namespace ryoji::allocators {
	template<size_t Capacity, size_t ObjectSize, uint8_t ObjectAlignment, class Allocator>
	class SlabAllocator
	{
		static_assert(Capacity != 0);
		static_assert(ObjectSize > 0, "ObjectSize is 0");
		static_assert(ObjectAlignment > 0, "ObjectAlignment is 0");
		static_assert(ObjectSize > sizeof(void**), "ObjectSize is too small to be contained by void**");
		
		struct null_deleter { template<typename T> void operator()(T*) {} };
		template<typename T> using move_ptr = std::unique_ptr<T, null_deleter>;


		SlabAllocator& operator=(const SlabAllocator&) = delete;
		SlabAllocator(const SlabAllocator&) = delete;
		SlabAllocator& operator=(SlabAllocator&&) = delete;

		Allocator allocator;
		void* memory = {};
		move_ptr<void*> freeList = nullptr;
		move_ptr<char> start = nullptr;

	public:
		SlabAllocator() {

			memory = allocator.allocate(Capacity, ObjectAlignment);

			assert(memory != nullptr);
			start.reset(reinterpret_cast<char*>(memory));

			uint8_t adjustment = zawarudo::pointer::getAlignForwardDiff(start.get(), ObjectAlignment);

			// save it as (void**)
			freeList.reset(reinterpret_cast<void**>(start.get() + adjustment));

			// Calculate the number of objects.
			size_t objectNum = (reinterpret_cast<uintptr_t>(start.get() + Capacity) - reinterpret_cast<uintptr_t>(start.get()) + adjustment) / ObjectSize;


			void** itr = freeList.get();
			for (size_t i = 0; i < objectNum - 1; ++i) {
				// calculate and store the address of the next item
				*itr = zawarudo::pointer::add(itr, ObjectSize);
				itr = reinterpret_cast<void**>(*itr);
			}

			*itr = nullptr;
		}

		~SlabAllocator() {
			allocator.deallocate(memory);
		}

		SlabAllocator(SlabAllocator&&) = default;

		void* allocate(size_t size, uint8_t alignment) noexcept
		{
			assert(size == ObjectSize);
			assert(alignment == ObjectAlignment);

			if (freeList == nullptr)
				return {};

			void* ret = reinterpret_cast<void*>(freeList);

			// advance the freeList to next object
			freeList = reinterpret_cast<void**>(*freeList);

			//++allocated;

			return ret;
		}

		void deallocate(void* blk) noexcept
		{
			assert(owns(blk));
			assert(reinterpret_cast<uintptr_t>(blk) % ObjectAlignment == 0);

			// set the value of p to free list's current value
			// We have to cast to void** because we can't dereference p into a void type, which makes no sense
			*(reinterpret_cast<void**>(blk)) = freeList;
			freeList = reinterpret_cast<void**>(blk);

			//--allocated;

		}

		bool owns(void* blk) const noexcept {
			return blk && blk >= start.get() && blk < start.get() + Capacity;
		}

		Allocator& getAllocator() {
			return allocator;
		}



	};


	template<size_t Capacity, size_t ObjectSize, uint8_t ObjectAlignment>
	using LocalSlabAllocator = SlabAllocator<Capacity, ObjectSize, ObjectAlignment, LocalAllocator<Capacity>>;

	template<size_t Capacity, size_t ObjectSize, uint8_t ObjectAlignment>
	using HeapSlabAllocator = SlabAllocator<Capacity, ObjectSize, ObjectAlignment, HeapAllocator>;
}

#endif 
