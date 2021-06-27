#ifndef __MOMO_C4f__
#define __MOMO_C4f__

struct c3f {
    union {
        f32 e[3];
        struct {
            f32 r, g, b;
        };
    };
};

struct c4f {
    union {
        f32 e[4];
        struct {
            union {
                v3f rgb;
                struct {
                    f32 r, g, b;
                };
            }; 
            f32 a;
        };
    };
    
    static inline c4f create(f32 r, f32 g, f32 b, f32 a);
};

inline c4f
c4f::create(f32 r, f32 g, f32 b, f32 a){
    return { r, g, b, a };
}

//TODO: idk, maybe just make a static function for each of these?
#define C4F_GREY1  c4f::create(0.1f, 0.1f, 0.1f, 1.f)
#define C4F_GREY2  c4f::create(0.2f, 0.2f, 0.2f, 1.f)
#define C4F_GREY3  c4f::create(0.3f, 0.3f, 0.3f, 1.f)
#define C4F_GREY4  c4f::create(0.4f, 0.4f, 0.4f, 1.f)
#define C4F_GREY5  c4f::create(0.5f, 0.5f, 0.5f, 1.f)
#define C4F_GREY6  c4f::create(0.6f, 0.6f, 0.6f, 1.f)
#define C4F_GREY7  c4f::create(0.7f, 0.7f, 0.7f, 1.f)
#define C4F_GREY8  c4f::create(0.8f, 0.8f, 0.8f, 1.f)
#define C4F_GREY9  c4f::create(0.9f, 0.9f, 0.9f, 1.f)
#define C4F_WHITE  c4f::create(1.f, 1.f, 1.f, 1.f)
#define C4F_BLACK  c4f::create(0.f, 0.f, 0.f, 1.f)
#define C4F_RED  c4f::create(1.f, 0.f, 0.f, 1.f)
#define C4F_GREEN  c4f::create(0.f, 1.f, 0.f, 1.f)
#define C4F_BLUE  c4f::create(0.f, 0.f, 1.f, 1.f)
#define C4F_YELLOW  c4f::create(1.f, 1.f, 0.f, 1.f)

#endif 
