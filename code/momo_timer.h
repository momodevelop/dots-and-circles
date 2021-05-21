#ifndef __MOMO_TIMER__
#define __MOMO_TIMER__

struct MM_Timer {
    f32 current;
    f32 end;
};

static inline MM_Timer
MM_Timer_Create(f32 end_time) {
    MM_Timer Ret = {};
    Ret.end = end_time;
    return Ret;
}

static inline void
MM_Timer_Tick(MM_Timer* t, f32 dt) {
    t->current += dt;
    if (t->current >= t->end) {
        t->current = t->end;
    }
}

static inline void
MM_Timer_Untick(MM_Timer* t, f32 dt) {
    t->current -= dt;
    if (t->current < 0.f ) {
        t->current = 0.f;
    }
}

static inline void
MM_Timer_Reset(MM_Timer* t) {
    t->current = 0.f;
}

static inline b8
MM_Timer_IsEnd(MM_Timer t) {
    return t.current >= t.end;
}

static inline b8
MM_Timer_IsBegin(MM_Timer t) {
    return t.current <= 0.f;
}

// Returns [0 - 1]
static inline f32
MM_Timer_Percent(MM_Timer t) {
    Assert(t.end != 0.f);
    return t.current / t.end;
}

#endif
