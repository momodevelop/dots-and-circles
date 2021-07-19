#ifndef __MOMO_MATH__
#define __MOMO_MATH__



// NOTE(Momo): To future self: We wrap the math functions IN CASE we 
// want to have specialized versions for different systems (using SSE etc)
//
// I guess a good rule of thumb is that the GAME CODE should never directly
// use standard library functions IN CASE we cannot use them!
//

// NOTE(Momo): Constants
#define PI 3.14159265358979323846264338327950288f
#define EPSILON 1.19209290E-07f
#define TAU PI * 2.f

// Get the Ratio of Value within the range [Min,Max] 
// Return value Will be [0, 1]
static inline f32
Ratio(f32 value, f32 min, f32 max) {
    return (value - min)/(max - min); 
}

// NOTE(Momo): Common Functions
static inline b8
IsEqual(f32 L, f32 R) {
    return ABS(L - R) <= EPSILON;
}

static inline f32 
DegToRad(f32 degrees) {
    return degrees * PI / 180.f;
}

static inline f32 
RadToDeg(f32 radians) {
    return radians * 180.f / PI;
}

static inline f32 
Sin(f32 x) {
    return sinf(x);
}

static inline f32 
Cos(f32 x) {
    return cosf(x);
}

static inline f32 
Tan(f32 x) {
    return tanf(x);
}


static inline f32 
Sqrt(f32 x) {
    return sqrtf(x);
}

static inline f32 
Asin(f32 x) {
    return asinf(x);
}

static inline f32 
Acos(f32 x) {
    return acosf(x);
}

static inline f32 
Atan(f32 x) {
    return atanf(x);
}

static inline f32
Pow(f32 b, f32 e) {
    return powf(b,e);
}


#include "momo_vector.h"
#include "momo_aabb.h"
#include "momo_circle.h"
#include "momo_matrix.h"
#include "momo_raylines.h"


//~ NOTE(Momo): Quad
// TODO(Momo): I don't think this belongs here?
// A bit to 'freeform' to actually use
struct quad2f {
    v2f Points[4];
};

// TODO(Momo): This definitely doesn't belong here
static inline quad2f
Quad2f_CreateDefaultUV() {
    quad2f Ret = {};
    Ret.Points[0] = { 0.f, 1.f };
    Ret.Points[1] = { 1.f, 1.f };
    Ret.Points[2] = { 1.f, 0.f };
    Ret.Points[3] = { 0.f, 0.f };
    
    return Ret;
}


static inline quad2f
Aabb2f_To_Quad2f(aabb2f Aabb) {
    quad2f Ret = {};
    Ret.Points[0] = { Aabb.min.x, Aabb.max.y };
    Ret.Points[1] = { Aabb.max.x, Aabb.max.y };
    Ret.Points[2] = { Aabb.max.x, Aabb.min.y };
    Ret.Points[3] = { Aabb.min.x, Aabb.min.y };
    
    return Ret;
}

#endif