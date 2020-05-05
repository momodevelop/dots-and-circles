#ifndef __RYOJI_UTILS_REFLECTION_H__
#define __RYOJI_UTILS_REFLECTION_H__

namespace ryoji::utils::reflection {
	// Generates a member function checker called 'prefix_memberFuncName'
	#define GENERATE_MEMBER_FUNCTION_PRODDER(prefix, memberFuncName, returnType, args) \
	template<typename T> \
	struct prefix##memberFuncName { \
		template<typename U, returnType##(U::*)##args>struct SFINAE {}; \
		template<typename U> static char Test(SFINAE<U, &U::##memberFuncName>*);\
		template<typename U> static int Test(...); \
		static const bool value = sizeof(Test<T>(nullptr)) == sizeof(char); \
	}; \
	template<typename T> \
	static constexpr bool prefix##memberFuncName##_v = prefix##memberFuncName##<T>::value;


}

#endif