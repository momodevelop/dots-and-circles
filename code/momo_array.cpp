
template<typename T>
auto&
Array<T>::operator[](u32 index) {
    Assert(index < count); 
    return data[index];
}

template<typename T>
b8
Array<T>::init(T* buffer, u32 count_) {
    if (!buffer || count_ == 0) {
        return false;
    }
    data = buffer;
    count = count_;
    return true;
}

template<typename T>
b8
Array<T>::alloc(Arena* arena, u32 count_) {
    T* buffer = arena->push_array<T>(count_);
    return init(buffer, count_);
}

template<typename T>
T*
Array<T>::get(u32 index) {
    if(index < count) {
        return data + index;
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

