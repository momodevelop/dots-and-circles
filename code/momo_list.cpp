
template<typename T>
T& 
List<T>::operator[](umi index) {
    ASSERT(index < count); 
    return data[index];
}


template<typename T>
b8
List<T>::init(T* data_in, umi cap_in) {
    if (!data_in || cap_in == 0) {
        return false;
    }
    this->data = data_in;
    this->cap = cap_in;
    this->count = 0;
    
    return true;
}

template<typename T>
b8
List<T>::alloc(Arena* arena, umi cap_in) {
    T* buffer = arena->push_array<T>(cap_in);
    if (!buffer) {
        return false;
    }
    return init(buffer, cap_in);
}



template<typename T>
void
List<T>::clear() {
    count = 0;
}

// NOTE(Momo): Push, but does not override whatever's in
// the container because user might have already initialized
// something important before that remained inside
template<typename T>
T*
List<T>::push() {
    if(count < cap) {
        T* ret = data + count++;
        return ret;
    }
    return nullptr;
}

template<typename T>
T*
List<T>::push_item(T item) {
    if(count < cap) {
        T* ret = data + count++;
        (*ret) = item;
        return ret;
    }
    return nullptr;
}

// NOTE(Momo): "Swap last element and remove"
template<typename T>
b8
List<T>::slear(umi index) {
    if (index < count) {
        data[index] = data[count-1];
        --count;
        return true;
    }
    else {
        return false;
    }
}

template<typename T>
b8
List<T>::pop() {
    if (count != 0) {
        --count;
        return true;
    }
    return false;
}

template<typename T>
T*
List<T>::last() {
    if (count == 0){
        return nullptr;
    }
    else {
        return get(count - 1);
    }
    
}

template<typename T>
umi
List<T>::remaining() {
    return cap - count;
}



template<typename T>
T*
List<T>::get(umi index) {
    if(index < count) {
        return data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
T* 
List<T>::operator+(umi index) {
    return get(index);
}

// TODO: refactor this into the class?
template<typename T>
template<typename Callback, typename... Args>
void
List<T>::foreach_slear_if(Callback callback, Args... args) {
    for (umi i = 0; i < count;) {
        if (callback(data + i, args...)) {
            slear(i);
            continue;
        }
        ++i;
    }
}

template<typename T>
template<typename Callback, typename... Args>
void
List<T>::foreach(Callback callback, Args... args) {
    for (umi i = 0; i < count; ++i) {
        callback(data + i, args...);
    }
}

//~ NOTE(Momo): Forward Iterator
template<typename T>
T& 
List_Forward_Itr<T>::operator->(){
    return list->data[index];
}

template<typename T>
List_Forward_Itr<T>
List<T>::begin() {
    return { this, 0 };
}

template<typename T>
List_Forward_Itr<T>
List<T>::end() {
    return { this, count - 1 };
}

template<typename T>
b8
List_Forward_Itr<T>::operator!=(List_Forward_Itr<T> rhs) {
    return index != rhs.index;
}

template<typename T>
List_Forward_Itr<T>&
List_Forward_Itr<T>::operator++() {
    ++itr.index;
    return itr;
}

template<typename T>
T&
List_Forward_Itr<T>::operator*() {
    return itr.list[itr.index];
}
//~ NOTE(Momo): Reverse Iterator
template<typename T>
T& 
List_Reverse_Itr<T>::operator->(){
    return list[index];
}

template<typename T>
List_Reverse_Itr<T>
List<T>::rbegin() {
    return { this, 0 };
}

template<typename T>
List_Reverse_Itr<T>
List<T>::rend() {
    return { this, count - 1 };
}

template<typename T>
b8
List_Reverse_Itr<T>::operator!=(List_Reverse_Itr<T> rhs) {
    return index != rhs.index;
}

template<typename T>
List_Reverse_Itr<T>&
List_Reverse_Itr<T>::operator++() {
    ++this->index;
    return (*this);
}


template<typename T>
T&
List_Reverse_Itr<T>::operator*() {
    umi actual_index = this->list->count - this->index - 1;
    return this->list[actual_index];
}

