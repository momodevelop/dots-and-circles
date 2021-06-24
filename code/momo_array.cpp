
template<typename T>
auto&
Array<T>::operator[](u32 index) {
    Assert(index < count); 
    return data[index];
}

template<typename T>
b8
Array<T>::init(T* buffer, u32 cap) {
    if (!buffer || count == 0) {
        return false;
    }
    A->data = buffer;
    A->count = count;
    return true;
}

template<typename T>
b8
Array<T>::alloc(Arena* arena, u32 count) {
    T* Buffer = arena->push_array<type>(count);
    return init(A, Buffer, count);
}

template<typename T>
T*
Array<T>::get(u32 index) {
    if(index < L->count) {
        return L->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
T* 
Array<T>::operator+(u32 index) {
    return get(index);
}

