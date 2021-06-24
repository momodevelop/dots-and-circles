/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename type>
struct array {
    type* Data;
    u32 Count;
    
    auto& operator[](u32 Index) {
        Assert(Index < Count); 
        return Data[Index];
    }
};

template<typename type>
static inline b8
Array_Init(array<type>* A, type* Buffer, u32 Count) {
    if (!Buffer || Count == 0) {
        return false;
    }
    A->Data = Buffer;
    A->Count = Count;
    return true;
}

template<typename type>
static inline b8
Array_New(array<type>* A, Arena* arena, u32 Count) {
    type* Buffer = arena->push_array<type>(Count);
    return Array_Init(A, Buffer, Count);
}

template<typename type>
static inline type*
Array_Get(array<type>* L, u32 Index) {
    if(Index < L->Count) {
        return L->Data + Index;
    }
    else {
        return nullptr;
    }
}

template<typename type>
static inline type* 
operator+(array<type> L, u32 Index) {
    return Array_Get(&L, Index);
}


#endif //MM_ARRAY_H
