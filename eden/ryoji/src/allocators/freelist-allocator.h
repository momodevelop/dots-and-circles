#ifndef __RYOJI_ALLOCATORS_FREELIST_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_FREELIST_ALLOCATOR_H__

// Simple allocator that reserves memory on the heap.
// Sometimes called the Mallocator.

#include <cassert>
#include <utility>
#include "blk.h"

#include "zawarudo/pointer.h"
#include "zawarudo/predef-freelist-strategies.h"
#include "local-allocator.h"
#include "heap-allocator.h"

namespace ryoji::allocators {

	template<size_t Capacity, class Allocator, class FitStrategy>
	class FreeListAllocator {
		static_assert(Capacity != 0);
	private:
		union FreeBlock {
			struct {
				size_t size;
				FreeBlock* next;
			};
		protected:
			max_align_t Align;
		};

		union Header {
			size_t size;
		protected:
			max_align_t Align;
		};

		class iterator {
			FreeBlock* current;
		public:
			iterator(FreeBlock* block) : current(block) {}
			iterator& operator++() {
				current = current->next;
				return (*this);
			}

			iterator& operator++(int) {
				iterator tmp(current);
				current = current->next;
				return tmp;
			}

			FreeBlock* operator*() {
				return current;
			}

			bool operator==(const iterator& rhs) {
				return this->current == rhs.current;
			}
			bool operator!=(const iterator& rhs) {
				return this->current != rhs.current;
			}

		};

		constexpr static size_t minBlockSize = sizeof(Header) > sizeof(FreeBlock) ? sizeof(Header) : sizeof(FreeBlock);

	public:
		Allocator allocator;

	public:
		FreeListAllocator()
		{
			memory = allocator.allocate(Capacity, alignof(max_align_t));
			assert(memory);
			start = static_cast<char*>(memory.ptr);

			this->freeList = reinterpret_cast<FreeBlock*>(start);
			this->freeList->size = Capacity;
			this->freeList->next = nullptr;

			// The whole allocator must be able to contain at least a minimum block size
			assert(freeList->size > minBlockSize);
		}

		~FreeListAllocator()
		{
			allocator.deallocate(memory);
		}

		Blk allocate(size_t size, uint8_t alignment) {
			assert(size && alignment);

			// Calculate the size of the header + object rounded to alignment.
			// All our objects and headers will be aligned to the maximum alignment size. 
			size_t roundObjectSize = zawarudo::pointer::roundToAlignment(size, alignof(max_align_t));
			size_t totalSize = sizeof(Header) + roundObjectSize;


			// result->first is previous node.
			// result->second is the node that fits.
			auto [prev, itr] = fitStrategy.find(iterator(freeList), iterator(nullptr), totalSize);


			if ((*itr) == nullptr)
				return {};



			// Could not find a block that fits

			// Here, we have found a block that fits and update our freeList.
			// Check if the block can be split after allocation.
			// It can be split if after allocation, it can store more than sizeof(Header)
			// Note: We don't have to calculate adjustment for the 'future' Header 
			// because our object and Header sizes are rounded to maximum alignment, so the adjustment
			// is already within totalSize.

			FreeBlock* nextBlock;
			size_t remainingSize = (*itr)->size - totalSize;
			if (remainingSize <= minBlockSize) {
				// if it's smaller or equal to the rounded Header size, 
				// future allocations in this block is impossible.

				// The next block would then be itr->next
				nextBlock = (*itr)->next;

				// let the total size be the whole block
				totalSize = (*itr)->size;
			}

			else {
				// Otherwise, create a new FreeBlock after the current block
				nextBlock = reinterpret_cast<FreeBlock*>(zawarudo::pointer::add((*itr), totalSize));
				nextBlock->size = remainingSize;
				nextBlock->next = (*itr)->next;

			}

			// Here, we update our linked list!
			// If there's a previous block, set it's next to itr->next
			if (*prev) {
				(*prev)->next = nextBlock;
			}
			// If there is no previous block, it means that itr is the head.
			// Set the head to the next block
			else {
				this->freeList = nextBlock;
			}

			// Get and Update the header
			Header* header = reinterpret_cast<Header*>(*itr);
			header->size = totalSize;

			// Get the object to return to the user
			void* ret = zawarudo::pointer::add(*itr, sizeof(Header));
			return { ret, size };
		}

		bool owns(Blk blk) const {
			return reinterpret_cast<char*>(blk.ptr) >= start && reinterpret_cast<char*>(blk.ptr) < start + Capacity;
		}



		void deallocate(Blk blk)
		{
			assert(blk);
			assert(owns(blk));

			Header* header = reinterpret_cast<Header*>(zawarudo::pointer::sub(blk.ptr, sizeof(Header)));
			uintptr_t blockEnd = reinterpret_cast<uintptr_t>(zawarudo::pointer::add(header, header->size));

			// Look for a FreeBlock which we can combine
			FreeBlock* itr = freeList;
			FreeBlock* prev = nullptr;
			while (itr != nullptr) {
				// search until we are past the current block
				if (reinterpret_cast<uintptr_t>(itr) >= blockEnd)
					break;
				prev = itr;
				itr = itr->next;
			}

			// if there is no prev block, that means itr is at the start of the block
			// set the head of freeList to be this 
			if (prev == nullptr) {
				// use prev for the next step of combining with the next block
				prev = reinterpret_cast<FreeBlock*>(header);
				prev->size = header->size;
				prev->next = freeList;
				freeList = prev;
			}

			else if (reinterpret_cast<uintptr_t>(prev) + prev->size == reinterpret_cast<uintptr_t>(header)) {
				// If the previous block is directly next to this block, combine by just adding this block's size 
				// to the prev block's size
				prev->size += header->size;
			}

			else {
				// Here, there is a prev block, but it is not next to the current block, so we can't combine
				// So we turn the current block into a Free
				FreeBlock* temp = reinterpret_cast<FreeBlock*>(header);
				// a bit of a tightrope here, but it should be fine because we have not touched the memory around header->size
				temp->size = header->size;
				temp->next = prev->next; // now, header->size is gone.

				// update prev block
				prev->next = temp;

				// set this for the next step of combining with the next block
				prev = temp;
			}

			// Check if we can combine prev with the NEXT block
			if (itr != nullptr && reinterpret_cast<uintptr_t>(itr) == blockEnd)
			{
				prev->size += itr->size;
				prev->next = itr->next;
			}
		}

	private:
		Blk memory = {};
		char* start = nullptr;
		FreeBlock* freeList = nullptr;

		FitStrategy fitStrategy;


	};


	template<size_t Capacity>
	using LocalFirstFitFreeListAllocator = FreeListAllocator<Capacity, LocalAllocator<Capacity>, zawarudo::FirstFitStrategy>;

	template<size_t Capacity>
	using HeapFirstFitFreeListAllocator = FreeListAllocator<Capacity, HeapAllocator, zawarudo::FirstFitStrategy>;

	template<size_t Capacity>
	using LocalBestFitFreeListAllocator = FreeListAllocator<Capacity, LocalAllocator<Capacity>, zawarudo::BestFitStrategy>;

	template<size_t Capacity>
	using HeapBestFitFreeListAllocator = FreeListAllocator<Capacity, HeapAllocator, zawarudo::BestFitStrategy>;
}

#endif 
