#ifndef __RYOJI_ALLOCATORS_SLAB_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_SLAB_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include "blk.h"

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
	public:
		Allocator allocator;

	public:
		SlabAllocator() {

			memory = allocator.allocate(Capacity, ObjectAlignment);

			assert(memory.ptr != nullptr);
			start = reinterpret_cast<char*>(memory.ptr);

			uint8_t adjustment = zawarudo::pointer::getAlignForwardDiff(start, ObjectAlignment);

			// save it as (void**)
			freeList = reinterpret_cast<void**>(start + adjustment);

			// Calculate the number of objects.
			size_t objectNum = (reinterpret_cast<uintptr_t>(start + Capacity) - reinterpret_cast<uintptr_t>(this->start) + adjustment) / ObjectSize;


			void** itr = freeList;
			for (size_t i = 0; i < objectNum - 1; ++i) {
				// calculate and store the address of the next item
				*itr = zawarudo::pointer::add(itr, ObjectSize);
				itr = reinterpret_cast<void**>(*itr);
			}

			*itr = nullptr;
			//allocated = 0;

		}

		~SlabAllocator() {
			allocator.deallocate(memory);
		}

		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size == ObjectSize);
			assert(alignment == ObjectAlignment);

			if (freeList == nullptr)
				return {};

			void* ret = reinterpret_cast<void*>(freeList);

			// advance the freeList to next object
			freeList = reinterpret_cast<void**>(*freeList);

			//++allocated;

			return { ret, size };
		}

		void deallocate(Blk blk)
		{
			assert(owns(blk));
			assert(reinterpret_cast<uintptr_t>(blk.ptr) % ObjectAlignment == 0);

			// set the value of p to free list's current value
			// We have to cast to void** because we can't dereference p into a void type, which makes no sense
			*(reinterpret_cast<void**>(blk.ptr)) = freeList;
			freeList = reinterpret_cast<void**>(blk.ptr);

			//--allocated;

		}

		bool owns(Blk blk) const noexcept {
			return blk && blk.ptr >= start && blk.ptr < start + Capacity;
		}

	private:
		Blk memory = {};
		void** freeList = nullptr;
		char* start = nullptr;

	};


	template<size_t Capacity, size_t ObjectSize, uint8_t ObjectAlignment>
	using LocalSlabAllocator = SlabAllocator<Capacity, ObjectSize, ObjectAlignment, LocalAllocator<Capacity>>;

	template<size_t Capacity, size_t ObjectSize, uint8_t ObjectAlignment>
	using HeapSlabAllocator = SlabAllocator<Capacity, ObjectSize, ObjectAlignment, HeapAllocator>;
}

#endif 
