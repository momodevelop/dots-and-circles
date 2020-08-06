#ifndef __RYOJI_HEADERS__
#define __RYOJI_HEADERS__

// Includes
#include <cstdint>

// Types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using usize = size_t;
using uptr = uintptr_t;
using b8 = bool;
#define pure static inline
#define global static
#define Abs(x) ((x < 0) ? -x : x)
#define Max(x, y) ((x > y) x : y)
#define Min(x, y) ((x < y) x : y)


#if ASSERT 
#include <assert.h>
#define Assert(x) assert(x)
#else
#define Assert(x) 
#endif


// Defer 
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

#define zawarudo_VARANON_IMPL(COUNTER) zawarudo_ryojianon##COUNTER
#define zawarudo_VARANON(counter) zawarudo_VARANON_IMPL(counter)
#define Defer auto zawarudo_VARANON(__COUNTER__) = zawarudo::defer_dummy{} + [&]()


#endif