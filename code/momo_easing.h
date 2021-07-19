#ifndef __MOMO_EASING_H__
#define __MOMO_EASING_H__


static inline
f32 EaseInSine(f32 t)  {
    return Sin(PI * 0.5f * t);
}


static inline
f32 EaseOutSine(f32 t) {
    return 1.0f + Sin(PI * 0.5f * (--t));
}

static inline
f32 EaseInOutSine(f32 t)  {
    return 0.5f * (1.f + Sin(PI * (t - 0.5f)));
}

static inline 
f32 EaseInQuad(f32 t)  {
    return t * t;
}

static inline f32 EaseOutQuad(f32 t)  {
    return t * (2.f -t);
}

static inline f32 EaseInOutQuad(f32 t)  {
    return t < 0.5f ? 2.f * t * t : t * (4.f -2.f * t) - 1.f;
}

static inline f32 EaseInCubic(f32 t)  {
    return t * t * t;
}

static inline f32 EaseOutCubic(f32 t)  {
    return 1.f + (t-1) * (t-1) * (t-1);
}

static inline f32 EaseInOutCubic(f32 t)  {
    return t < 0.5f ? 4.f * t * t * t : 1.f + (t-1) * (2.f * (t-2)) * (2.f * (t-2));
}

static inline f32 EaseInQuart(f32 t)  {
    t *= t;
    return t * t;
}

static inline f32 EaseOutQuart(f32 t) {
    --t;
    t = t * t;
    return 1.f - t * t;
}

static inline f32 EaseInOutQuart(f32 t)  {
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

static inline f32 EaseInQuint(f32 t)  {
    f32 t2 = t * t;
    return t * t2 * t2;
}

static inline f32 EaseOutQuint(f32 t)  {
    --t;
    f32 t2 = t * t;
    return 1.f +t * t2 * t2;
}

static inline f32 EaseInOutQuint(f32 t)  {
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



static inline f32 EaseInCirc(f32 t)  {
    return 1.f -Sqrt(1.f -t);
}

static inline f32 EaseOutCirc(f32 t)  {
    return Sqrt(t);
}

static inline f32 EaseInOutCirc(f32 t)  {
    if (t < 0.5f) {
        return (1.f -Sqrt(1.f -2.f * t)) * 0.5f;
    }
    else {
        return (1.f +Sqrt(2.f * t - 1.f)) * 0.5f;
    }
}

static inline f32 EaseInBack(f32 t)  {
    return t * t * (2.7f * t - 1.7f);
}

static inline 
f32 EaseOutBack(f32 t)  {
    --t;
    return 1.f + t * t * (2.7f * t + 1.7f);
}

static inline 
f32 EaseInOutBack(f32 t)  {
    if (t < 0.5f) {
        return t * t * (7.f * t - 2.5f) * 2.f;
    }
    else {
        --t;
        return 1.f + t * t * 2.f * (7.f * t + 2.5f);
    }
}

static inline 
f32 EaseInElastic(f32 t)  {
    f32 t2 = t * t;
    return t2 * t2 * Sin(t * PI * 4.5f);
}

static inline 
f32 EaseOutElastic(f32 t)  {
    f32 t2 = (t - 1.f) * (t - 1.f);
    return 1.f -t2 * t2 * Cos(t * PI * 4.5f);
}

static inline
f32 EaseInOutElastic(f32 t)  {
    f32 t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8.f * t2 * t2 * Sin(t * PI * 9.f);
    }
    else if (t < 0.55f) {
        return 0.5f +0.75f * Sin(t * PI * 4.f);
    }
    else {
        t2 = (t - 1.f) * (t - 1.f);
        return 1.f -8.f * t2 * t2 * Sin(t * PI * 9.f);
    }
}



// NOTE(Momo): These require power function. 
static inline f32 
EaseInBounce(f32 t)  {
    return Pow(2.f, 6.f * (t - 1.f)) * Abs(Sin(t * PI * 3.5f));
}


static inline 
f32 EaseOutBounce(f32 t) {
    return 1.f -Pow(2.f, -6.f * t) * Abs(Cos(t * PI * 3.5f));
}

static inline 
f32 EaseInOutBounce(f32 t) {
    if (t < 0.5f) {
        return 8.f * Pow(2.f, 8.f * (t - 1.f)) * Abs(Sin(t * PI * 7.f));
    }
    else {
        return 1.f -8.f * Pow(2.f, -8.f * t) * Abs(Sin(t * PI * 7.f));
    }
}

static inline f32 EaseInExpo(f32 t)  {
    return (Pow(2.f, 8.f * t) - 1.f) / 255.f;
}

static inline f32 EaseOutExpo(f32 t)  {
    return t == 1.f ? 1.f : 1.f -Pow(2.f, -10.f * t);
}

static inline f32 EaseInOutExpo(f32 t)  {
    if (t < 0.5f) {
        return (Pow(2.f, 16.f * t) - 1.f) / 510.f;
    }
    else {
        return 1.f -0.5f * Pow(2.f, -16.f * (t - 0.5f));
    }
}

#endif 
