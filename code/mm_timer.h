#ifndef __MOMO_TIMER__
#define __MOMO_TIMER__

struct timer {
    f32 Current;
    f32 End;
};

static inline timer
CreateTimer(f32 EndTime) {
    timer Ret = {};
    Ret.End = EndTime;
    return Ret;
}

static inline void
Tick(timer* Timer, f32 DeltaTime) {
    Timer->Current += DeltaTime;
    if (Timer->Current >= Timer->End) {
        Timer->Current = Timer->End;
    }
}

static inline void
Untick(timer* Timer, f32 DeltaTime) {
    Timer->Current -= DeltaTime;
    if (Timer->Current < 0.f ) {
        Timer->Current = 0.f;
    }
}

static inline void
Reset(timer* Timer) {
    Timer->Current = 0.f;
}

static inline b32
IsEnd(timer Timer) {
    return Timer.Current >= Timer.End;
}

static inline b32
IsBegin(timer Timer) {
    return Timer.Current <= 0.f;
}

// Returns [0 - 1]
static inline f32
Percent(timer Timer) {
    Assert(Timer.End != 0.f);
    return Timer.Current / Timer.End;
}

#endif
