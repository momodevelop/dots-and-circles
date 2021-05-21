#ifndef __MOMO_EASING_H__
#define __MOMO_EASING_H__


static inline f32 
MM_Ease_InSine(f32 t)  {
    return MM_Math_Sin(MM_Math_Pi32 * 0.5f * t);
}


static inline f32 
MM_Ease_OutSine(f32 t) {
    return 1.0f + MM_Math_Sin(MM_Math_Pi32 * 0.5f * (--t));
}

static inline f32 
MM_Ease_InOutSine(f32 t)  {
    return 0.5f * (1.f + MM_Math_Sin(MM_Math_Pi32 * (t - 0.5f)));
}

static inline f32 
MM_Ease_InQuad(f32 t)  {
    return t * t;
}

static inline f32 
MM_Ease_OutQuad(f32 t)  {
    return t * (2.f -t);
}

static inline f32
MM_Ease_InOutQuad(f32 t)  {
    return t < 0.5f ? 2.f * t * t : t * (4.f -2.f * t) - 1.f;
}

static inline f32 
MM_Ease_InCubic(f32 t)  {
    return t * t * t;
}

static inline f32 
MM_Ease_OutCubic(f32 t)  {
    return 1.f + (t-1) * (t-1) * (t-1);
}

static inline f32 
MM_Ease_InOutCubic(f32 t)  {
    return t < 0.5f ? 4.f * t * t * t : 1.f + (t-1) * (2.f * (t-2)) * (2.f * (t-2));
}

static inline f32
MM_Ease_InQuart(f32 t)  {
    t *= t;
    return t * t;
}

static inline f32
MM_Ease_OutQuart(f32 t) {
    --t;
    t = t * t;
    return 1.f - t * t;
}

static inline f32 
MM_Ease_InOutQuart(f32 t)  {
    if (t < 0.5f) {
        t *= t;
        return 8.f * t * t;
    }
    else {
        --t;
        t = t * t;
        return 1.f -8.f * t * t;
    }
}

static inline f32 
MM_Ease_InQuint(f32 t)  {
    f32 t2 = t * t;
    return t * t2 * t2;
}

static inline f32 
MM_Ease_OutQuint(f32 t)  {
    --t;
    f32 t2 = t * t;
    return 1.f +t * t2 * t2;
}

static inline f32 
MM_Ease_InOutQuint(f32 t)  {
    f32 t2;
    if (t < 0.5f) {
        t2 = t * t;
        return 16.f * t * t2 * t2;
    }
    else {
        --t;
        t2 = t * t;
        return 1.f +16.f * t * t2 * t2;
    }
}



static inline f32 
MM_Ease_InCirc(f32 t)  {
    return 1.f -MM_Math_Sqrt(1.f -t);
}

static inline f32 
MM_Ease_OutCirc(f32 t)  {
    return MM_Math_Sqrt(t);
}

static inline f32 
MM_Ease_InOutCirc(f32 t)  {
    if (t < 0.5f) {
        return (1.f -MM_Math_Sqrt(1.f -2.f * t)) * 0.5f;
    }
    else {
        return (1.f +MM_Math_Sqrt(2.f * t - 1.f)) * 0.5f;
    }
}

static inline f32 
MM_Ease_InBack(f32 t)  {
    return t * t * (2.7f * t - 1.7f);
}

static inline 
f32 MM_Ease_OutBack(f32 t)  {
    --t;
    return 1.f + t * t * (2.7f * t + 1.7f);
}

static inline 
f32 MM_Ease_InOutBack(f32 t)  {
    if (t < 0.5f) {
        return t * t * (7.f * t - 2.5f) * 2.f;
    }
    else {
        --t;
        return 1.f + t * t * 2.f * (7.f * t + 2.5f);
    }
}

static inline f32 
MM_Ease_InElastic(f32 t)  {
    f32 t2 = t * t;
    return t2 * t2 * MM_Math_Sin(t * MM_Math_Pi32 * 4.5f);
}

static inline f32 
MM_Ease_OutElastic(f32 t)  {
    f32 t2 = (t - 1.f) * (t - 1.f);
    return 1.f -t2 * t2 * MM_Math_Cos(t * MM_Math_Pi32 * 4.5f);
}

static inline f32
MM_Ease_InOutElastic(f32 t)  {
    f32 t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8.f * t2 * t2 * MM_Math_Sin(t * MM_Math_Pi32 * 9.f);
    }
    else if (t < 0.55f) {
        return 0.5f +0.75f * MM_Math_Sin(t * MM_Math_Pi32 * 4.f);
    }
    else {
        t2 = (t - 1.f) * (t - 1.f);
        return 1.f -8.f * t2 * t2 * MM_Math_Sin(t * MM_Math_Pi32 * 9.f);
    }
}



// NOTE(Momo): These require power function. 
static inline f32 
MM_Ease_InBounce(f32 t)  {
    return MM_Math_Pow(2.f, 6.f * (t - 1.f)) * AbsOf(MM_Math_Sin(t * MM_Math_Pi32 * 3.5f));
}


static inline 
f32 MM_Ease_OutBounce(f32 t) {
    return 1.f -MM_Math_Pow(2.f, -6.f * t) * AbsOf(MM_Math_Cos(t * MM_Math_Pi32 * 3.5f));
}

static inline 
f32 MM_Ease_InOutBounce(f32 t) {
    if (t < 0.5f) {
        return 8.f * MM_Math_Pow(2.f, 8.f * (t - 1.f)) * AbsOf(MM_Math_Sin(t * MM_Math_Pi32 * 7.f));
    }
    else {
        return 1.f -8.f * MM_Math_Pow(2.f, -8.f * t) * AbsOf(MM_Math_Sin(t * MM_Math_Pi32 * 7.f));
    }
}

static inline f32 MM_Ease_InExpo(f32 t)  {
    return (MM_Math_Pow(2.f, 8.f * t) - 1.f) / 255.f;
}

static inline f32 MM_Ease_OutExpo(f32 t)  {
    return t == 1.f ? 1.f : 1.f -MM_Math_Pow(2.f, -10.f * t);
}

static inline f32 MM_Ease_InOutExpo(f32 t)  {
    if (t < 0.5f) {
        return (MM_Math_Pow(2.f, 16.f * t) - 1.f) / 510.f;
    }
    else {
        return 1.f -0.5f * MM_Math_Pow(2.f, -16.f * (t - 0.5f));
    }
}

#endif 
