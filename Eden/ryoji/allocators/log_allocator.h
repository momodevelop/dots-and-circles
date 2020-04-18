#ifndef __RYOJI_ALLOCATORS_LOG_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_LOG_ALLOCATOR_H__

// Allocator that logs to std::cout whenever there's allocator and deallocation (for debugging purposes)
#include "blk.h"

#include <iostream>

namespace ryoji::allocators {
	namespace zawarudo {
		class BasicLogStrategy {
		public:
			void printAllocate(Blk blk) {
				std::cout << "Allocating: " << blk.size << " @ " << blk.ptr << std::endl;
			}
			void printDeallocate(Blk blk) {
				std::cout << "Deallocating: " << blk.size << " @ " << blk.ptr << std::endl;
			}
		};
	}


	template<class Allocator, class LogStrategy = zawarudo::BasicLogStrategy>
	class LogAllocator
	{
		Allocator allocator;
		LogStrategy logStrategy;
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			assert(size && alignment);

			Blk ret = allocator.allocate(size, alignment);
			logStrategy.printAllocate(ret);
			return ret;
		}

		void deallocate(Blk blk)
		{
			if (!blk)
				return;

			assert(owns(blk));

			logStrategy.printDeallocate(blk);
			allocator.deallocate(blk);
		}

		bool owns(Blk blk) const noexcept {
			return allocator.owns(blk);
		}

		void deallocateAll() noexcept {
			allocator.deallocateAll();
		}
	
	};
}

#endif