/* date = June 27th 2021 3:54 pm */

#ifndef MOMO_MATRIX_H
#define MOMO_MATRIX_H

union m22f {
    v2f elements[2];
    inline v2f& operator[](u32 i);
    
    static inline m22f create_rotation(f32 rad);
    static inline m22f create_identity();
};

union m44f {
    v4f elements[4];
    
    static inline m44f create_identity();
    static inline m44f create_rotation_x(f32 rad);
    static inline m44f create_rotation_y(f32 rad);
    static inline m44f create_rotation_z(f32 rad);
    
    static inline m44f create_translation(f32 x, f32 y, f32 z);
    static inline m44f create_scale(f32 x, f32 y, f32 z);
    static inline m44f create_translation(v3f v);
    static inline m44f create_scale(v3f v);
    
    static inline m44f create_orthographic(f32 ndc_left,
                                           f32 ndc_right,
                                           f32 ndc_bottom,
                                           f32 ndc_top,
                                           f32 ndc_near,
                                           f32 ndc_far,
                                           f32 left,
                                           f32 right,
                                           f32 bottom,
                                           f32 top,
                                           f32 near,
                                           f32 far,
                                           b8 flip_z);
    inline v4f& operator[](u32 i);
    
};

//~ NOTE(Momo): m22f
inline v2f&
m22f::operator[](u32 i){
    ASSERT(i < 2); 
    return elements[i]; 
}

static inline m22f 
concat(m22f lhs, m22f rhs) {
    m22f ret = {};
    for (u8 r = 0; r < 2; r++) { 
        for (u8 c = 0; c < 2; c++) { 
            for (u8 i = 0; i < 2; i++) 
                ret[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return ret;
}

static inline v2f
concat(m22f l, v2f r) {
    v2f ret = {};
    ret[0] = l[0][0] * r[0] + l[0][1] * r[1];
    ret[1] = l[1][0] * r[0] + l[1][1] * r[1];
    
    return ret;
}

static inline m22f 
operator*(m22f l, m22f r) {
    return concat(l,r);
}

static inline v2f 
operator*(m22f l, v2f r) {
    return concat(l,r);
}

inline m22f 
m22f::create_identity() {
    m22f ret = {};
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    
    return ret;
}


inline m22f 
m22f::create_rotation(f32 rad) {
    // NOTE(Momo): 
    // c -s
    // s  c
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m22f ret = {};
    ret[0][0] = c;
    ret[0][1] = -s;
    ret[1][0] = s;
    ret[1][1] = c;
    
    return ret;
}

//~ NOTE(Momo): m44f
inline v4f& 
m44f::operator[](u32 i) { 
    ASSERT(i < 4); 
    return elements[i]; 
}
static inline m44f 
concat(m44f lhs, m44f rhs) {
    m44f ret = {};
    for (u8 r = 0; r < 4; r++) { 
        for (u8 c = 0; c < 4; c++) { 
            for (u8 i = 0; i < 4; i++) 
                ret[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return ret;
}

static inline m44f 
operator*(m44f l, m44f r) {
    return concat(l,r);
}

inline m44f 
m44f::create_identity() {
    m44f ret = {};
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    ret[2][2] = 1.f;
    ret[3][3] = 1.f;
    
    return ret;
}

static inline m44f 
transpose(m44f m) {
    m44f ret = {};
    for (int i = 0; i < 4; ++i ) {
        for (int j = 0; j < 4; ++j) {
            ret[i][j] = m[j][i];
        }
    }
    
    return ret;
}


inline m44f 
m44f::create_translation(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    // 1 0 0 x
    // 0 1 0 y
    // 0 0 1 z
    // 0 0 0 1
    //
    m44f ret = create_identity();
    ret[0][3] = x;
    ret[1][3] = y;
    ret[2][3] = z;
    
    return ret;
}


inline m44f
m44f::create_translation(v3f v) {
    return create_translation(v.x, v.y, v.z);; 
}

inline m44f 
m44f::create_rotation_x(f32 rad) {
    // NOTE(Momo): 
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m44f ret = {};
    ret[0][0] = 1.f;
    ret[3][3] = 1.f;
    ret[1][1] = c;
    ret[1][2] = -s;
    ret[2][1] = s;
    ret[2][2] = c;
    
    return ret;
}

inline m44f 
m44f::create_rotation_y(f32 rad) {
    // NOTE(Momo): 
    //  c  0  s  0
    //  0  1  0  0
    // -s  0  c  0
    //  0  0  0  1
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m44f Ret = {};
    Ret[0][0] = c;
    Ret[0][2] = s;
    Ret[1][1] = 1.f;
    Ret[2][0] = -s;
    Ret[2][2] = c;
    Ret[3][3] = 1.f;
    
    return Ret;
}

inline m44f 
m44f::create_rotation_z(f32 rad) {
    // NOTE(Momo): 
    //  c -s  0  0
    //  s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    
    f32 c = cos(rad);
    f32 s = sin(rad);
    m44f Ret = {};
    Ret[0][0] = c;
    Ret[0][1] = -s;
    Ret[1][0] = s;
    Ret[1][1] = c;
    Ret[2][2] = 1.f;
    Ret[3][3] = 1.f;
    
    return Ret;
}

inline m44f
m44f::create_scale(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    //  x  0  0  0
    //  0  y  0  0
    //  0  0  z  0
    //  0  0  0  1
    m44f ret = {};
    ret[0][0] = x;
    ret[1][1] = y;
    ret[2][2] = z;
    ret[3][3] = 1.f;
    
    return ret; 
}

inline m44f
m44f::create_scale(v3f v) {
    return create_scale(v.x, v.y, v.z);; 
}

inline m44f 
m44f::create_orthographic(f32 ndc_left, f32 ndc_right,
                          f32 ndc_bottom, f32 ndc_top,
                          f32 ndc_near, f32 ndc_far,
                          f32 left, f32 right, 
                          f32 bottom, f32 top,
                          f32 near, f32 far,
                          b8 flip_z) 
{
    m44f ret = {};
    ret[0][0] = (ndc_right-ndc_left)/(right-left);
    ret[1][1] = (ndc_top-ndc_bottom)/(top-bottom);
    ret[2][2] = (flip_z ? -1.f : 1.f) * (ndc_far-ndc_near)/(far-near);
    ret[3][3] = 1.f;
    ret[0][3] = -(right+left)/(right-left);
    ret[1][3] = -(top+bottom)/(top-bottom);
    ret[2][3] = -(far+near)/(far-near);
    
    return ret;
}

#endif //MOMO_MATRIX_H