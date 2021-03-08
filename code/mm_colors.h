#ifndef __MOMO_COLOR__
#define __MOMO_COLOR__

struct c3f {
    union {
        f32 E[3];
        struct {
            f32 R, G, B;
        };
    };
};

struct c4f {
    union {
        f32 E[4];
        struct {
            union {
                v3f RGB;
                struct {
                    f32 R, G, B;
                };
            }; 
            f32 A;
        };
    };
};

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
