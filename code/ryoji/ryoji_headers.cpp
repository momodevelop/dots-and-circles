#ifndef __RYOJI_HEADERS__
#define __RYOJI_HEADERS__

// Includes
#include <cstdint>
#include <cmath>
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


// Shortern terms
#define pure static inline

// Assertion
#if ASSERT 
#include <assert.h>
#define Assert(x) assert(x)
#else
#define Assert(x) 
#endif


#endif