#ifndef __RYOJI_MATH_ZAWARUDO_CONSTANTS_H__
#define __RYOJI_MATH_ZAWARUDO_CONSTANTS_H__


#ifndef MAKE_CONSTEXPR_FLOAT
#define MAKE_CONSTEXPR_FLOAT(whole, decimal)\
template<typename T> constexpr auto __##whole##p##decimal() {\
	if constexpr (std::is_same_v<T, float>) { return whole##.##decimal##f; }\
	else if constexpr (std::is_same_v<T, double>) { return whole##.##decimal; }\
	else if constexpr (std::is_same_v<T, long double>) { return whole##.##decimal##L; }\
	else static_assert(zawarudo::always_false_v<T>);\
}\
template<typename T> constexpr static auto _##whole##p##decimal = __##whole##p##decimal<T>(); 
#endif

#ifndef MAKE_CONSTEXPR_FLOAT_NAMED
#define MAKE_CONSTEXPR_FLOAT_NAMED(name, value) \
template<typename T> constexpr auto _##name() { \
	if constexpr (std::is_same_v<T, float>) { return value ## f; }\
    else if constexpr (std::is_same_v<T, double>) { return value; }\
    else if constexpr (std::is_same_v<T, long double>) { return value ## L; }\
    else static_assert(zawarudo::always_false_v<T>); \
}\
template<typename T> constexpr static auto name = _##name<T>();
#endif


namespace ryoji::math {
	namespace zawarudo {
		template<class T> struct always_false : std::false_type {};
		template<class T> struct always_true : std::true_type {};
		template<class T> constexpr static auto always_false_v = always_false<T>::value;
		template<class T> constexpr static auto always_true_v = always_true<T>::value;


		MAKE_CONSTEXPR_FLOAT(180, 0);
		MAKE_CONSTEXPR_FLOAT(1, 0);
		MAKE_CONSTEXPR_FLOAT_NAMED(pi, 3.14159265358979323846);
		MAKE_CONSTEXPR_FLOAT_NAMED(zero, 0.0);
	}
    
}


#undef MAKE_CONSTEXPR_FLOAT_NAMED
#undef MAKE_CONSTEXPR_FLOAT
#endif // __MATH_INTERNAL_CONSTANTS_H__