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
                MM_V3f RGB;
                struct {
                    f32 R, G, B;
                };
            }; 
            f32 A;
        };
    };
};

static inline c4f
C4f_Create(f32 R, f32 G, f32 B, f32 A) {
    c4f Ret = {};
    Ret.R = R;
    Ret.G = G;
    Ret.B = B;
    Ret.A = A;
    
    return Ret;
}

#define Color_Grey1  C4f_Create(0.1f, 0.1f, 0.1f, 1.f)
#define Color_Grey2  C4f_Create(0.2f, 0.2f, 0.2f, 1.f)
#define Color_Grey3  C4f_Create(0.3f, 0.3f, 0.3f, 1.f)
#define Color_Grey4  C4f_Create(0.4f, 0.4f, 0.4f, 1.f)
#define Color_Grey5  C4f_Create(0.5f, 0.5f, 0.5f, 1.f)
#define Color_Grey6  C4f_Create(0.6f, 0.6f, 0.6f, 1.f)
#define Color_Grey7  C4f_Create(0.7f, 0.7f, 0.7f, 1.f)
#define Color_Grey8  C4f_Create(0.8f, 0.8f, 0.8f, 1.f)
#define Color_Grey9  C4f_Create(0.9f, 0.9f, 0.9f, 1.f)
#define Color_White  C4f_Create(1.f, 1.f, 1.f, 1.f)
#define Color_Black  C4f_Create(0.f, 0.f, 0.f, 1.f)
#define Color_Red  C4f_Create(1.f, 0.f, 0.f, 1.f)
#define Color_Green  C4f_Create(0.f, 1.f, 0.f, 1.f)
#define Color_Blue  C4f_Create(0.f, 0.f, 1.f, 1.f)
#define Color_Yellow  C4f_Create(1.f, 1.f, 0.f, 1.f)

#endif 
