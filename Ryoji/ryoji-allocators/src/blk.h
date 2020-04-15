#ifndef __RYOJI_ALLOCATORS_ALLOCATORS_BLK_H__
#define __RYOJI_ALLOCATORS_ALLOCATORS_BLK_H__

namespace ryoji::allocators {
	struct Blk {
		void* ptr;
		size_t size;

		Blk(void * ptr = nullptr, size_t size = 0) : ptr(ptr), size(size) {}

		bool operator==(const Blk& rhs) const {
			return ptr == rhs.ptr && size == rhs.size;
		}

		operator bool() const {
			return size != 0;
		}

	};
}


#endif