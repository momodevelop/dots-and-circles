#ifndef __RYOJI_DEFER__
#define __RYOJI_DEFER__

template<class F> struct ScopeGuard {
    F f;
    ~ScopeGuard() { f(); }
};


namespace zawarudo {
    struct defer_dummy {
    };
    
    template<class F> ScopeGuard<F> operator+(defer_dummy, F f) {
        return { f };
    }
}

#define zawarudo_VARANON_IMPL(LINE) zawarudo_ryojianon##LINE
#define zawarudo_VARANON(line) zawarudo_VARANON_IMPL(line)
#define defer auto zawarudo_VARANON(__LINE__) = ryoji::zawarudo::defer_dummy{} + [&]()


#endif