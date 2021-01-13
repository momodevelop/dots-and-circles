#ifndef __MOMO_COLOR__
#define __MOMO_COLOR__

#include "mm_maths.h"


#define GenerateSubscriptOp(Amt) inline auto& operator[](usize I) { Assert(I < Amt); return Elements[I]; }

template<typename t, usize N>
struct color {
    t Elements[N];
    GenerateSubscriptOp(N)
};

template<typename t>
struct color<t,3> {
    union {
        t Elements[3];
        struct {
            t R, G, B;
        };
    };
};

template<typename t>
struct color<t,4> {
    union {
        t Elements[4];
        struct {
            union {
                vec<t,3> RGB;
                struct {
                    t R, G, B;
                };
            }; 
            t A;
        };
    };
};
using c4f = color<f32,4>;

static c4f Color_Grey1 = { 0.1f, 0.1f, 0.1f, 1.f };
static c4f Color_Grey2 = { 0.2f, 0.2f, 0.2f, 1.f };
static c4f Color_Grey3 = { 0.3f, 0.3f, 0.3f, 1.f };
static c4f Color_Grey4 = { 0.4f, 0.4f, 0.4f, 1.f };
static c4f Color_Grey5 = { 0.5f, 0.5f, 0.5f, 1.f };
static c4f Color_Grey6 = { 0.6f, 0.6f, 0.6f, 1.f };
static c4f Color_Grey7 = { 0.7f, 0.7f, 0.7f, 1.f };
static c4f Color_Grey8 = { 0.8f, 0.8f, 0.8f, 1.f };
static c4f Color_Grey9 = { 0.9f, 0.9f, 0.9f, 1.f };
static c4f Color_White = { 1.f, 1.f, 1.f, 1.f };
static c4f Color_Black = { 0.f, 0.f, 0.f, 1.f };
static c4f Color_Red = { 1.f, 0.f, 0.f, 1.f };
static c4f Color_Green = { 0.f, 1.f, 0.f, 1.f };
static c4f Color_Blue = { 0.f, 0.f, 1.f, 1.f };
static c4f Color_Yellow = { 1.f, 1.f, 0.f, 1.f };


#endif 
