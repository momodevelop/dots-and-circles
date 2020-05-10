#ifndef __RYOJI_ALLOCATORS_MIDDLEWARE_ALLOCATOR_H__
#define __RYOJI_ALLOCATORS_MIDDLEWARE_ALLOCATOR_H__

#include "blk.h"
namespace ryoji::allocators {

	namespace zawarudo {
		#define GENERATE_HAS_MEMBER_FUNCTION_CHECKER(prefix, memberFuncName, returnType, args) \
		template<typename T> \
		struct prefix##memberFuncName { \
			template<typename U, returnType##(U::*)##args>struct SFINAE {}; \
			template<typename U> static char Test(SFINAE<U, &U::##memberFuncName>*);\
			template<typename U> static int Test(...); \
			static const bool value = sizeof(Test<T>(nullptr)) == sizeof(char); \
		};\
		template<typename T> \
		static constexpr bool prefix##memberFuncName##_v = prefix##memberFuncName##<T>::value;

		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preAllocate, void, (size_t, uint8_t));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postAllocate, void, (Blk));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preDeallocate, void, (Blk));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postDeallocate, void, (Blk));


		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preAllocateR, void, (size_t&, uint8_t&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postAllocateR, void, (Blk&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preDeallocateR, void, (Blk&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postDeallocateR, void, (Blk&));

		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preAllocateCR, void, (const size_t&, const uint8_t&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postAllocateCR, void, (const Blk&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, preDeallocateCR, void, (const Blk&));
		GENERATE_HAS_MEMBER_FUNCTION_CHECKER(has_, postDeallocateCR, void, (const Blk&));

		template<typename T> static constexpr bool has_any_preAllocate_v =		has_preAllocate_v<T> || has_preAllocateCR_v<T> || has_preAllocateR_v<T>; 
		template<typename T> static constexpr bool has_any_postAllocate_v =		has_postAllocate_v<T> || has_postAllocateCR_v<T> || has_postAllocateR_v<T>;
		template<typename T> static constexpr bool has_any_preDeallocate_v =	has_preDeallocate_v<T> || has_preDeallocateCR_v<T> || has_preDeallocateR_v<T>;
		template<typename T> static constexpr bool has_any_postDeallocate_v =	has_postDeallocate_v<T> || has_postDeallocateCR_v<T> || has_postDeallocateR_v<T>;

		// should I care about volatile?
		#undef GENERATE_HAS_MEMBER_FUNCTION_CHECKER
	}

	template<class Allocator, class Middleware>
	class MiddlewareAllocator
	{
		Allocator allocator;
		Middleware middleware;
	public:
		Blk allocate(size_t size, uint8_t alignment)
		{
			using namespace zawarudo;
			assert(size && alignment);

			if constexpr (has_any_preAllocate_v<Middleware>) 
				middleware.preAllocate(size, alignment);

			Blk ret = allocator.allocate(size, alignment);

			if constexpr (has_any_postAllocate_v<Middleware>)
				middleware.postAllocate(ret);

			return ret;
		}

		void deallocate(Blk blk)
		{
			using namespace zawarudo;
			if (!blk)
				return;

			assert(owns(blk));


			if constexpr (has_any_preDeallocate_v<Middleware>)
				middleware.preDeallocate(blk);

			allocator.deallocate(blk);
			
			if constexpr (has_any_postDeallocate_v<Middleware>)
				middleware.postDeallocate(blk);
		}

		bool owns(Blk blk) const noexcept {
			return allocator.owns(blk);
		}

		Allocator& getAllocator() {
			return allocator;
		}

		Middleware& getMiddleware() {
			return middleware;
		}

		
	};
}

#endif