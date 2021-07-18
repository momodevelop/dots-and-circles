#ifndef __MOMO_EASING_H__
#define __MOMO_EASING_H__


static inline
f32 ease_in_sine(f32 t)  {
    return Sin(PI * 0.5f * t);
}


static inline
f32 ease_out_sine(f32 t) {
    return 1.0f + Sin(PI * 0.5f * (--t));
}

static inline
f32 EaseInOutSine(f32 t)  {
    return 0.5f * (1.f + Sin(PI * (t - 0.5f)));
}

static inline 
f32 ease_in_quad(f32 t)  {
    return t * t;
}

static inline f32 ease_out_quad(f32 t)  {
    return t * (2.f -t);
}

static inline f32 ease_in_out_quad(f32 t)  {
    return t < 0.5f ? 2.f * t * t : t * (4.f -2.f * t) - 1.f;
}

static inline f32 ease_in_cubic(f32 t)  {
    return t * t * t;
}

static inline f32 ease_out_cubic(f32 t)  {
    return 1.f + (t-1) * (t-1) * (t-1);
}

static inline f32 ease_in_out_cubic(f32 t)  {
    return t < 0.5f ? 4.f * t * t * t : 1.f + (t-1) * (2.f * (t-2)) * (2.f * (t-2));
}

static inline f32 ease_in_quart(f32 t)  {
    t *= t;
    return t * t;
}

static inline f32 ease_out_quart(f32 t) {
    --t;
    t = t * t;
    return 1.f - t * t;
}

static inline f32 ease_in_out_quart(f32 t)  {
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

static inline f32 ease_in_quint(f32 t)  {
    f32 t2 = t * t;
    return t * t2 * t2;
}

static inline f32 ease_out_quint(f32 t)  {
    --t;
    f32 t2 = t * t;
    return 1.f +t * t2 * t2;
}

static inline f32 ease_in_out_quint(f32 t)  {
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



static inline f32 ease_in_circ(f32 t)  {
    return 1.f -Sqrt(1.f -t);
}

static inline f32 ease_out_circ(f32 t)  {
    return Sqrt(t);
}

static inline f32 ease_in_out_circ(f32 t)  {
    if (t < 0.5f) {
        return (1.f -Sqrt(1.f -2.f * t)) * 0.5f;
    }
    else {
        return (1.f +Sqrt(2.f * t - 1.f)) * 0.5f;
    }
}

static inline f32 ease_in_back(f32 t)  {
    return t * t * (2.7f * t - 1.7f);
}

static inline 
f32 ease_out_back(f32 t)  {
    --t;
    return 1.f + t * t * (2.7f * t + 1.7f);
}

static inline 
f32 ease_in_out_back(f32 t)  {
    if (t < 0.5f) {
        return t * t * (7.f * t - 2.5f) * 2.f;
    }
    else {
        --t;
        return 1.f + t * t * 2.f * (7.f * t + 2.5f);
    }
}

static inline 
f32 ease_in_elastic(f32 t)  {
    f32 t2 = t * t;
    return t2 * t2 * Sin(t * PI * 4.5f);
}

static inline 
f32 ease_out_elastic(f32 t)  {
    f32 t2 = (t - 1.f) * (t - 1.f);
    return 1.f -t2 * t2 * Cos(t * PI * 4.5f);
}

static inline
f32 ease_in_out_elastic(f32 t)  {
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
ease_in_bounce(f32 t)  {
    return Pow(2.f, 6.f * (t - 1.f)) * ABS(Sin(t * PI * 3.5f));
}


static inline 
f32 ease_out_bounce(f32 t) {
    return 1.f -Pow(2.f, -6.f * t) * ABS(Cos(t * PI * 3.5f));
}

static inline 
f32 ease_in_out_bounce(f32 t) {
    if (t < 0.5f) {
        return 8.f * Pow(2.f, 8.f * (t - 1.f)) * ABS(Sin(t * PI * 7.f));
    }
    else {
        return 1.f -8.f * Pow(2.f, -8.f * t) * ABS(Sin(t * PI * 7.f));
    }
}

static inline f32 ease_in_expo(f32 t)  {
    return (Pow(2.f, 8.f * t) - 1.f) / 255.f;
}

static inline f32 ease_out_expo(f32 t)  {
    return t == 1.f ? 1.f : 1.f -Pow(2.f, -10.f * t);
}

static inline f32 ease_in_out_expo(f32 t)  {
    if (t < 0.5f) {
        return (Pow(2.f, 16.f * t) - 1.f) / 510.f;
    }
    else {
        return 1.f -0.5f * Pow(2.f, -16.f * (t - 0.5f));
    }
}

#endif 
