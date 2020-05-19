#ifndef __RYOJI_ALLOCATORS_STACK_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_STACK_ALLOCATOR_H__


#include <cassert>

#include "zawarudo/pointer.h"

#include "local-allocator.h"
#include "heap-allocator.h"

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
		struct null_deleter { template<typename T> void operator()(T*) {} };
		template<typename T> using move_ptr = std::unique_ptr<T, null_deleter>;

		struct Header {
			uint8_t adjustment; // The adjustment value should not be bigger than uint8_t 
		};
		StackAllocator& operator=(const StackAllocator&) = delete;
		StackAllocator(const StackAllocator&) = delete;
		StackAllocator& operator=(StackAllocator&&) = delete;

		Allocator allocator;
		void* memoryBlock = {};
		move_ptr<char> start{ nullptr };
		move_ptr<char> current{ nullptr };
		move_ptr<char> metadataCurrent{ nullptr }; // Pointer to the start of metadata of headers

	public:
		StackAllocator()
		{
			memoryBlock = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memoryBlock);
			start.reset(reinterpret_cast<char*>(memoryBlock));
			current.reset(reinterpret_cast<char*>(memoryBlock));

			// Make sure the metadata starts at an piece of memory that it aligns to.
			// From here on, our Headers will be contiguously lined up :)
			metadataCurrent = move_ptr<char>(zawarudo::pointer::getAlignBackward(start.get() + Capacity, alignof(Header)));
		}

		~StackAllocator()
		{
			allocator.deallocate(memoryBlock);
		}

		StackAllocator(StackAllocator&&) = default;

		void* allocate(size_t size, uint8_t alignment) noexcept
		{
			assert(size != 0);
			assert(alignment != 0);

			uint8_t adjustment = zawarudo::pointer::getAlignForwardDiff(current.get(), alignment);

			// Make sure that there is space to alloc 
			if (current.get() + adjustment + size > metadataCurrent.get() - sizeof(Header)) {
				return nullptr;
			}

			// alloc for header
			metadataCurrent.reset(metadataCurrent.get() - sizeof(Header));
			reinterpret_cast<Header*>(metadataCurrent.get())->adjustment = adjustment;

			// alloc for data, just like linear allocation
			char* alignedAddress = current.get() + adjustment;
			current.reset(alignedAddress + size);

			// returns the address for the user
			return alignedAddress;
		}


		bool owns(void* blk) const noexcept {
			return blk && blk >= start.get() && blk < start.get() + Capacity;
		}


		void deallocate(void* blk) noexcept
		{
			assert(owns(blk));

			// When you free, get the adjustment from the current Header to know how much to fall back 
			current.reset(reinterpret_cast<char*>(blk) - reinterpret_cast<Header*>(metadataCurrent.get())->adjustment);

			// And move the metadataCurrent forward
			metadataCurrent.reset(metadataCurrent.get() + sizeof(Header));
		}

		Allocator& getAllocator() {
			return allocator;
		}

	};

	template<size_t Capacity>
	using LocalStackAllocator = StackAllocator<Capacity, LocalAllocator<Capacity>>;

	template<size_t Capacity>
	using HeapStackAllocator = StackAllocator<Capacity, HeapAllocator>;

}


#endif