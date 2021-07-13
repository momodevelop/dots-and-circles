
template<typename T>
auto&
Array<T>::operator[](umi index) {
    ASSERT(index < count); 
    return data[index];
}

template<typename T>
b8
Array<T>::init(T* buffer, umi count_) {
    if (!buffer || count_ == 0) {
        return false;
    }
    data = buffer;
    count = count_;
    return true;
}

template<typename T>
b8
Array<T>::alloc(Arena* arena, umi count_) {
    T* buffer = arena->push_array<T>(count_);
    return init(buffer, count_);
}

template<typename T>
T*
Array<T>::get(umi index) {
    if(index < count) {
        return data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
T* 
Array<T>::operator+(umi index) {
    return get(index);
}

