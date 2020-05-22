#ifndef __RYOJI_ALLOCATORS_ALLOCATORS_BLK_H__
#define __RYOJI_ALLOCATORS_ALLOCATORS_BLK_H__

namespace ryoji::allocators {
	struct Blk {
		void* ptr;
		size_t size;

		Blk(void * ptr = nullptr, size_t size = 0) noexcept : ptr(ptr), size(size) {}

		bool operator==(const Blk& rhs) const noexcept {
			return ptr == rhs.ptr && size == rhs.size;
		}

		operator bool() const noexcept  {
			return ptr != nullptr && size != 0;
		}

		Blk& operator=(const Blk& blk) = default;
		Blk(const Blk& blk) = default;

		Blk& operator=(Blk&& movee) noexcept {
			if (this != &movee) {
				this->ptr = movee.ptr;
				this->size = movee.size;
				movee.ptr = nullptr;
				movee.size = 0;
			}
			return *this;
		}

		Blk(Blk&& movee) noexcept : ptr(movee.ptr), size(movee.size) {
			movee.ptr = nullptr;
			movee.size = 0;
		}


	};
}


#endif