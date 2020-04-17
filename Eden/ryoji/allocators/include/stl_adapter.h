#ifndef __RYOJI_ALLOCATORS_STL_ADAPTER_H__
#define __RYOJI_ALLOCATORS_STL_ADAPTER_H__


// https://howardhinnant.github.io/allocator_boilerplate.html
// For use in STL containers. Kind of an adaptor for the rest of the allocators.

namespace ryoji::allocators {
	template<typename T, class Allocator>
	class STLAdapter : private Allocator
	{
	public:
		using value_type = T;
		STLAdapter() = default;
		template <class U> STLAdapter(STLAdapter<U, Allocator> const&) noexcept {}

	public:
		value_type* allocate(size_t size)
		{
			return static_cast<value_type*>(Allocator::allocate(size * sizeof(value_type), alignof(value_type)).ptr);
		}

		void deallocate(value_type* p, size_t size) noexcept 
		{
			Allocator::deallocate({ p, size });
		}

	};

	template <class Allocator, class T, class U>
	bool operator==(STLAdapter<T, Allocator> const&, STLAdapter<U, Allocator> const&) noexcept
	{
		return true;
	}

	template <class Allocator, class T, class U>
	bool operator!=(STLAdapter<T, Allocator> const& x, STLAdapter<U, Allocator> const& y) noexcept
	{
		return !(x == y);
	}
}
#endif 