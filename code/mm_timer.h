#ifndef __MOMO_TIMER__
#define __MOMO_TIMER__

#include "mm_core.h"

struct timer {
    f32 Current;
    f32 End;
};

static inline timer
Timer(f32 Duration) {
    timer Ret = {};
    Ret.End = Duration;
    return Ret;
}

static inline void
Tick(timer* Timer, f32 DeltaTime) {
    Timer->Current += DeltaTime;
}

static inline void
Reset(timer* Timer) {
    Timer->Current = 0.f;
}

static inline b32
IsTimeUp(timer Timer) {
    return Timer.Current >= Timer.End;
}


#endif
