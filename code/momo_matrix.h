/* date = June 27th 2021 3:54 pm */

#ifndef MOMO_MATRIX_H
#define MOMO_MATRIX_H

struct m22f {
    v2f elements[2];
    inline v2f& operator[](u32 i);
};

struct m44f {
    v4f elements[4];
    inline v4f& operator[](u32 i);
    
};

//~ NOTE(Momo): m22f
v2f&
m22f::operator[](u32 i){
    ASSERT(i < 2); 
    return elements[i]; 
}

static inline m22f 
Concat(m22f lhs, m22f rhs) {
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
Concat(m22f l, v2f r) {
    v2f ret = {};
    ret[0] = l[0][0] * r[0] + l[0][1] * r[1];
    ret[1] = l[1][0] * r[0] + l[1][1] * r[1];
    
    return ret;
}

static inline m22f 
operator*(m22f l, m22f r) {
    return Concat(l,r);
}

static inline v2f 
operator*(m22f l, v2f r) {
    return Concat(l,r);
}

m22f 
m22f_Identity() {
    m22f ret = {};
    ret[0][0] = 1.f;
    ret[1][1] = 1.f;
    
    return ret;
}


inline m22f 
m22f_Rotation(f32 rad) {
    // NOTE(Momo): 
    // c -s
    // s  c
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
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

static inline v4f
Concat(m44f l, v4f r) {
    v4f ret = {};
    ret[0] = l[0][0] * r[0] + l[0][1] * r[1] + l[0][2] * r[2] + l[0][3] * r[3];
    ret[1] = l[1][0] * r[0] + l[1][1] * r[1] + l[1][2] * r[2] + l[1][3] * r[3];
    ret[2] = l[2][0] * r[0] + l[2][1] * r[1] + l[2][2] * r[2] + l[2][3] * r[3];
    ret[3] = l[3][0] * r[0] + l[3][1] * r[1] + l[3][2] * r[2] + l[3][3] * r[3];
    
    return ret;
}

static inline m44f 
Concat(m44f lhs, m44f rhs) {
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
    return Concat(l,r);
}

static inline v4f 
operator*(m44f l, v4f r) {
    return Concat(l,r);
}

inline m44f 
m44f_Identity() {
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
m44f_Translation(f32 x, f32 y, f32 z) {
    // NOTE(Momo): 
    // 1 0 0 x
    // 0 1 0 y
    // 0 0 1 z
    // 0 0 0 1
    //
    m44f ret = m44f_Identity();
    ret[0][3] = x;
    ret[1][3] = y;
    ret[2][3] = z;
    
    return ret;
}


inline m44f
m44f_Translation(v3f v) {
    return m44f_Translation(v.x, v.y, v.z);
}

inline m44f 
m44f_RotationX(f32 rad) {
    // NOTE(Momo): 
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
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
m44f_RotationY(f32 rad) {
    // NOTE(Momo): 
    //  c  0  s  0
    //  0  1  0  0
    // -s  0  c  0
    //  0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
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
m44f_RotationZ(f32 rad) {
    // NOTE(Momo): 
    //  c -s  0  0
    //  s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
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
m44f_Scale(f32 x, f32 y, f32 z) {
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
m44f_Scale(v3f v) {
    return m44f_Scale(v.x, v.y, v.z);; 
}

inline m44f 
m44f_Orthographic(f32 left, f32 right, 
                  f32 bottom, f32 top,
                  f32 near, f32 far) 
{
    m44f ret = {};
    ret[0][0] = 2.f/(right-left);
    ret[1][1] = 2.f/(top-bottom);
    ret[2][2] = -2.f/(far-near);
    ret[3][3] = 1.f;
    ret[0][3] = -(right+left)/(right-left);
    ret[1][3] = -(top+bottom)/(top-bottom);
    ret[2][3] = -(far+near)/(far-near);
    
    return ret;
}

m44f 
m44f_Frustum(f32 left, f32 right, 
             f32 bottom, f32 top,
             f32 near, f32 far) 
{
    m44f ret = {};
    ret[0][0] = (2.f*near)/(right-left);
    ret[1][1] = (2.f*near)/(top-bottom);
    ret[2][2] = -(far+near)/(far-near);
    ret[3][2] = 1.f;
    ret[0][3] = -near*(right+left)/(right-left);
    ret[1][3] = -near*(top+bottom)/(top-bottom);
    ret[2][3] = 2.f*far*near/(near-far);
    
    return ret;
}

m44f 
m44f_Perspective(f32 fov, f32 aspect,
                 f32 near, f32 far) 
{
    f32 top = near * Tan(fov*0.5f);
    f32 right = top * aspect;
    return m44f_Frustum(-right, right,
                        -top, top,
                        near, far);
}


#endif //MOMO_MATRIX_H
