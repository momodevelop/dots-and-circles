#ifndef __RYOJI_DEFER_H__
#define __RYOJI_DEFER_H__

namespace ryoji {
	namespace zawarudo {
		template<class F> struct deferrer {
			F f;
			~deferrer() { f(); }
		};


		struct defer_dummy {};
		template<class F>
		deferrer<F> operator+(defer_dummy, F f) {
			return { f };
		}
	}
}
#define zawarudo_VARANON_IMPL(LINE) zawarudo_ryojianon##LINE
#define zawarudo_VARANON(line) zawarudo_VARANON_IMPL(line)
#define defer auto zawarudo_VARANON(__LINE__) = ryoji::zawarudo::defer_dummy{} + [&]()


#endif