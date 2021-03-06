#ifndef __MOMO_TIMER__
#define __MOMO_TIMER__

struct Timer {
    f32 current;
    f32 end;
    
    static inline Timer create(f32 duration);
    inline void tick(f32 dt);
    inline void untick(f32 dt);
    inline void reset();
    inline b8 is_end();
    inline b8 is_begin();
    inline f32 percent();
};

inline Timer
Timer::create(f32 duration) {
    ASSERT(!IsEqual(duration, 0.f));
    Timer ret = {};
    ret.end = duration;
    return ret;
}

void
Timer::tick(f32 dt) {
    current += dt;
    if (current >= end) {
        current = end;
    }
}

void
Timer::untick(f32 dt) {
    current -= dt;
    if (current < 0.f ) {
        current = 0.f;
    }
}

void
Timer::reset() {
    current = 0.f;
}

b8
Timer::is_end() {
    return current >= end;
}

b8
Timer::is_begin() {
    return current <= 0.f;
}

// Returns [0 - 1]
f32
Timer::percent() {
    return current / end;
}

#endif
