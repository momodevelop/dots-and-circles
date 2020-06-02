#ifndef __RYOJI_HEADERS__
#define __RYOJI_HEADERS__

// Includes
#include <cstdint>


// TODO(Momo): Write our own cos/sin?
#include <cmath>

// TODO(Momo): write our own epsilon?
#include <limits>

// Types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using usize = size_t;
using uptr = uintptr_t;

#define pure static inline
#define Abs(x) ((x < 0) ? -x : x)
#define Max(x, y) ((x > y) x : y)
#define Min(x, y) ((x < y) x : y)


// Assertion
// TODO(Momo): make macros purple
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

#define zawarudo_VARANON_IMPL(LINE) zawarudo_ryojianon##LINE
#define zawarudo_VARANON(line) zawarudo_VARANON_IMPL(line)
#define defer auto zawarudo_VARANON(__LINE__) = zawarudo::defer_dummy{} + [&]()


#endif