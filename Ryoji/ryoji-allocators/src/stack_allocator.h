#ifndef __RYOJI_ALLOCATORS_STACK_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_STACK_ALLOCATOR_H__


#include <cassert>
#include "blk.h"
#include "detail/pointer.h"

#include "local_allocator.h"
#include "heap_allocator.h"

namespace ryoji::allocators {
	// The Stack Base needs to be freed in the reverse order of its allocation.
	// For each block of memory allocd from the top of the given memory, 
	// there will be a Header metadata allocd at the bottom of the given memory.
	// 
	// The Header will store the space between each data (due to how alignment works)
	// ----------------------------------------------------------------------------------------- 
	// | data | | data | data | | | data |                         |header|header|header|header| 
	// -----------------------------------------------------------------------------------------
	//        ^^^ space
	// ^ start                           ^ current                 ^metaDataCurrent            ^ end
	//

	template <size_t Capacity, class Allocator>
	class StackAllocator {
		static_assert(Capacity != 0);
		// The adjustment value should not be bigger than uint8_t 
		struct Header {
			uint8_t adjustment;
		};
		Allocator allocator;
		Blk memoryBlock = {};
		char* start = nullptr;
		char* current = nullptr;
		char* metadataCurrent = nullptr; // Pointer to the start of metadata of headers
	public:
		StackAllocator()
		{
			memoryBlock = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memoryBlock);
			start = reinterpret_cast<char*>(memoryBlock.ptr);
			deallocateAll();
		}

		~StackAllocator()
		{
			allocator.deallocate(memoryBlock);
		}

		Blk allocate(size_t size, uint8_t alignment) noexcept
		{
			assert(size != 0);
			assert(alignment != 0);

			uint8_t adjustment = detail::pointer::getAlignForwardDiff(current, alignment);

			// Make sure that there is space to alloc 
			if (current + adjustment + size > metadataCurrent - sizeof(Header)) {
				return {};
			}

			// alloc for header
			this->metadataCurrent -= sizeof(Header);
			reinterpret_cast<Header*>(this->metadataCurrent)->adjustment = adjustment;

			// alloc for data, just like linear allocation
			char* alignedAddress = this->current + adjustment;
			this->current = alignedAddress + size;

			// returns the address for the user
			return { alignedAddress, size };
		}


		bool owns(Blk blk) const noexcept {
			return blk.ptr >= start && blk.ptr < start + Capacity;
		}


		void deallocate(Blk blk)  noexcept
		{
			if (!blk)
				return;

			assert(owns(blk));

			// When you free, get the adjustment from the current Header to know how much to fall back 
			this->current = reinterpret_cast<char*>(blk.ptr) - reinterpret_cast<Header*>(this->metadataCurrent)->adjustment;

			// And move the metadataCurrent forward
			this->metadataCurrent += sizeof(Header);
		}

		void deallocateAll() noexcept {
			current = start;

			// Make sure the metadata starts at an piece of memory that it aligns to.
			// From here on, our Headers will be contiguously lined up :)
			this->metadataCurrent = detail::pointer::getAlignBackward(start + Capacity, alignof(Header));
		}


	};

	template<size_t Capacity>
	using LocalStackAllocator = StackAllocator<Capacity, LocalAllocator<Capacity>>;

	template<size_t Capacity>
	using HeapStackAllocator = StackAllocator<Capacity, HeapAllocator>;

}


#endif