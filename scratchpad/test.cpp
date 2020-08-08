
#include "../code/ryoji.h"

template<usize N = 3, typename T = float>
struct vec {
    T E[N];
    inline f32 operator[](usize index) const {
        Assert(index < N);
        return E[index];
    };
    inline f32& operator[](usize index) { 
        Assert(index < N);
        return E[index]; 
    }
};

// TODO(Momo): Templates....?
template<usize N, typename T>
static inline vec<N,T> 
Add(vec<N,T> L, vec<N,T> R) {
    vec<N,T> Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret[i] = L[i] + R[i];
    return Ret;
}

template<usize N, typename T>
static inline vec<N,T> 
Sub(vec<N,T> L, vec<N,T> R) {
    vec<N,T> Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret[i] = L[i] - R[i];
    return Ret;
}

template<usize N, typename T>
static inline vec<N,T> 
Mul(vec<N,T> L, T R) {
    vec<N,T> Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret[i] = L[i] * R;
    return Ret;
}

template<usize N, typename T>
static inline vec<N,T> 
Div(vec<N,T> L, T R) {
    vec<N,T> Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret[i] = L[i] / R;
    return Ret;
}

template<usize N, typename T>
static inline vec<N,T> 
Negate(vec<N,T> L){
    vec<N,T> Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret[i] = -Ret[i];
    return Ret;
}

template<usize N, typename T>
static inline bool 
IsEqual(vec<N,T> L, vec<N,T> R) {
    return 
        IsEqual(L.X, R.X) && 
        IsEqual(L.Y, R.Y) && 
        IsEqual(L.Z, R.Z);
}
template<usize N, typename T>
static inline f32 
Dot(vec<N,T> L, vec<N,T> R) {
    T Ret = {};
    for(usize i = 0; i < N; ++i ) 
        Ret += L[i] * R[i] ;
    return Ret;
}

template<usize N, typename T>
static inline vec<N,T> 
operator+(vec<N,T> L, vec<N,T> R)  { 
    return Add(L, R); 
}

template<usize N, typename T>
static inline vec<N,T> 
operator-(vec<N,T> L, vec<N,T> R)  { 
    return Sub(L, R);
}

template<usize N, typename T>
static inline vec<N,T> 
operator*(vec<N,T> L, f32 R)  { 
    return Mul(L, R);
}

template<usize N, typename T>
static inline vec<N,T> 
operator*(float L, vec<N,T> R)  { 
    return Mul(R, L);
}

template<usize N, typename T>
static inline f32 
operator*(vec<N,T> L, vec<N,T> R) {
    return Dot(L, R); 
}

template<usize N, typename T>
static inline vec<N,T> 
operator/(vec<N,T> L, f32 R)  { 
    return Div(L, R); 
}

template<usize N, typename T>
static inline vec<N,T>& 
operator+=(vec<N,T>& L, vec<N,T> R) {
    return L = L + R;
}

template<usize N, typename T>
static inline vec<N,T>& 
operator-=(vec<N,T>& L, vec<N,T> R) {
    return L = L - R;
}

template<usize N, typename T>
static inline vec<N,T>& 
operator*=(vec<N,T>& L, f32 R) {
    return L = L * R;
}

template<usize N, typename T>
static inline vec<N,T>& 
operator/=(vec<N,T>& L, f32 R) {
    return L = L / R;
}

template<usize N, typename T>
static inline bool 
operator==(vec<N,T> L, vec<N,T> R)  { 
    return IsEqual(L, R);
}

template<usize N, typename T>
static inline bool 
operator!=(vec<N,T> L, vec<N,T> R) { 
    return !(L == R); 
}

template<usize N, typename T>
static inline vec<N,T> 
operator-(vec<N,T> L)  {  
    return { -L.X, -L.Y, -L.Z}; 
}



// NOTE(Momo): explicit template instantiation?


int main() {
    vec<3, f32> Vec1 = {};
    vec<3, f32> Vec2 = {};
    
    Add(Vec1, Vec2);
    
}