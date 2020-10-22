#ifndef __MOMO_EASING_H__
#define __MOMO_EASING_H__

#include "mm_std.h"

#include "mm_maths.h"

static inline
f32 mmes_EaseInSine(f32 t)  {
    return mmm_Sin(mmm_Pi32 * 0.5f * t);
}


static inline
f32 mmes_EaseOutSine(f32 t) {
    return 1.0f + mmm_Sin(mmm_Pi32 * 0.5f * (--t));
}

static inline
f32 mmes_EaseInOutSine(f32 t)  {
    return 0.5f * (1.f + mmm_Sin(mmm_Pi32 * (t - 0.5f)));
}

static inline 
f32 mmes_EaseInQuad(f32 t)  {
    return t * t;
}

static inline f32 mmes_EaseOutQuad(f32 t)  {
    return t * (2.f -t);
}

static inline f32 mmes_EaseInOutQuad(f32 t)  {
    return t < 0.5f ? 2.f * t * t : t * (4.f -2.f * t) - 1.f;
}

static inline f32 mmes_EaseInCubic(f32 t)  {
    return t * t * t;
}

static inline f32 mmes_EaseOutCubic(f32 t)  {
    return 1.f +(--t) * t * t;
}

static inline f32 mmes_EaseInOutCubic(f32 t)  {
    return t < 0.5f ? 4.f * t * t * t : 1.f +(--t) * (2.f * (--t)) * (2.f * t);
}

static inline f32 mmes_EaseInQuart(f32 t)  {
    t *= t;
    return t * t;
}

static inline f32 mmes_EaseOutQuart(f32 t)  {
    t = (--t) * t;
    return 1.f -t * t;
}

static inline f32 mmes_EaseInOutQuart(f32 t)  {
    if (t < 0.5f) {
        t *= t;
        return 8.f * t * t;
    }
    else {
        t = (--t) * t;
        return 1.f -8.f * t * t;
    }
}

static inline f32 mmes_EaseInQuint(f32 t)  {
    f32 t2 = t * t;
    return t * t2 * t2;
}

static inline f32 mmes_EaseOutQuint(f32 t)  {
    f32 t2 = (--t) * t;
    return 1.f +t * t2 * t2;
}

static inline f32 mmes_EaseInOutQuint(f32 t)  {
    f32 t2;
    if (t < 0.5f) {
        t2 = t * t;
        return 16.f * t * t2 * t2;
    }
    else {
        t2 = (--t) * t;
        return 1.f +16.f * t * t2 * t2;
    }
}



static inline f32 mmes_EaseInCirc(f32 t)  {
    return 1.f -mmm_Sqrt(1.f -t);
}

static inline f32 mmes_EaseOutCirc(f32 t)  {
    return mmm_Sqrt(t);
}

static inline f32 mmes_EaseInOutCirc(f32 t)  {
    if (t < 0.5f) {
        return (1.f -mmm_Sqrt(1.f -2.f * t)) * 0.5f;
    }
    else {
        return (1.f +mmm_Sqrt(2.f * t - 1.f)) * 0.5f;
    }
}

static inline f32 mmes_EaseInBack(f32 t)  {
    return t * t * (2.7f * t - 1.7f);
}

static inline 
f32 mmes_EaseOutBack(f32 t)  {
    return 1.f +(--t) * t * (2.7f * t + 1.7f);
}

static inline 
f32 mmes_EaseInOutBack(f32 t)  {
    if (t < 0.5f) {
        return t * t * (7.f * t - 2.5f) * 2.f;
    }
    else {
        return 1.f +(--t) * t * 2.f * (7.f * t + 2.5f);
    }
}

static inline 
f32 mmes_EaseInElastic(f32 t)  {
    f32 t2 = t * t;
    return t2 * t2 * mmm_Sin(t * mmm_Pi32 * 4.5f);
}

static inline 
f32 mmes_EaseOutElastic(f32 t)  {
    f32 t2 = (t - 1.f) * (t - 1.f);
    return 1.f -t2 * t2 * mmm_Cos(t * mmm_Pi32 * 4.5f);
}

static inline
f32 mmes_EaseInOutElastic(f32 t)  {
    f32 t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8.f * t2 * t2 * mmm_Sin(t * mmm_Pi32 * 9.f);
    }
    else if (t < 0.55f) {
        return 0.5f +0.75f * mmm_Sin(t * mmm_Pi32 * 4.f);
    }
    else {
        t2 = (t - 1.f) * (t - 1.f);
        return 1.f -8.f * t2 * t2 * mmm_Sin(t * mmm_Pi32 * 9.f);
    }
}



// NOTE(Momo): These require power function. 
static inline f32 
mmes_EaseInBounce(f32 t)  {
    return mmm_Pow(2.f, 6.f * (t - 1.f)) * Abs(mmm_Sin(t * mmm_Pi32 * 3.5f));
}


static inline 
f32 mmes_EaseOutBounce(f32 t) {
    return 1.f -mmm_Pow(2.f, -6.f * t) * Abs(mmm_Cos(t * mmm_Pi32 * 3.5f));
}

static inline 
f32 mmes_EaseInOutBounce(f32 t) {
    if (t < 0.5f) {
        return 8.f * mmm_Pow(2.f, 8.f * (t - 1.f)) * Abs(mmm_Sin(t * mmm_Pi32 * 7.f));
    }
    else {
        return 1.f -8.f * mmm_Pow(2.f, -8.f * t) * Abs(mmm_Sin(t * mmm_Pi32 * 7.f));
    }
}

static inline f32 mmes_EaseInExpo(f32 t)  {
    return (mmm_Pow(2.f, 8.f * t) - 1.f) / 255.f;
}

static inline f32 mmes_EaseOutExpo(f32 t)  {
    return t == 1.f ? 1.f : 1.f -mmm_Pow(2.f, -10.f * t);
}

static inline f32 mmes_EaseInOutExpo(f32 t)  {
    if (t < 0.5f) {
        return (mmm_Pow(2.f, 16.f * t) - 1.f) / 510.f;
    }
    else {
        return 1.f -0.5f * mmm_Pow(2.f, -16.f * (t - 0.5f));
    }
}

#endif 
