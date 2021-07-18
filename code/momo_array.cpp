
template<typename T>
inline auto&
Array<T>::operator[](u32 index) {
    ASSERT(index < count); 
    return data[index];
}

template<typename T>
static inline b8
Array_Init(Array<T>* a, T* buffer, u32 count) {
    if (!buffer || count == 0) {
        return false;
    }
    a->data = buffer;
    a->count = count;
    return true;
}

template<typename T>
static inline b8
Array_Alloc(Array<T>* a, Arena* arena, u32 count) {
    T* buffer = Arena_Push_Array<T>(arena, count);
    return Array_Init(a, buffer, count);
}

template<typename T>
static inline T*
Array_Get(Array<T>* a, u32 index) {
    if(index < a->count) {
        return a->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
static inline T* 
operator+(Array<T> a, u32 index) {
    return Array_Get(&a, index);
}

