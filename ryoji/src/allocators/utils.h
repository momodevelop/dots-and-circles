#ifndef __RYOJI_ALLOCATORS_UTILS_H__
#define __RYOJI_ALLOCATORS_UTILS_H__

#include <utility>
namespace ryoji::allocators {
	template<typename T, typename Allocator, typename ... Args>
	T* construct(Allocator& allocator, Args&&... args) {
		void* ptr = allocator.allocate(sizeof(T), alignof(T));
		return new (ptr) T(std::forward<Args>(args)...);
	}

	template<typename T, typename Allocator>
	void destruct(Allocator& allocator, T* obj) {
		obj->~T();
		allocator.deallocate(obj);
	}
}
#endif 