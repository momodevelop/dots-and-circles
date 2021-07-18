
template<typename T>
b8
Queue<T>::init(T* buffer, u32 cap_in) {
    if (!buffer || cap_in < 0) {
        return false;
    }
    start = finish = cap_in;
    cap = cap_in;
    data = buffer;
    
    return true;
}

template<typename T>
b8
Queue<T>::alloc(Arena* arena, u32 cap_in) {
    T* buffer = Arena_Push_Array<T>(arena, cap_in);
    if (!buffer) {
        return false;
    }
    
    return init(buffer, cap_in);
}

template<typename T>
b8 
Queue<T>::is_empty() {
    return start == cap || 
        finish == cap;
}

template<typename T>
b8
Queue<T>::is_full() {
    b8 normal_case = (start == 0 && finish == cap-1);
    b8 backward_case = finish == (start-1);
    
    return !is_empty() && (normal_case || backward_case);
}

template<typename T>
T* 
Queue<T>::push() {
    if (is_full()) {
        return nullptr;
    }
    else if (is_empty()) {
        start = finish = 0;
    }
    else if (finish == cap - 1) {
        // finish is already at the back of the array
        finish = 0;
    }
    else {
        // Normal case: just advance finish
        ++finish;
    }
    return data + finish;
}

template<typename T>
T*
Queue<T>::next() {
    if (is_empty()) {
        return nullptr;
    }
    return data + start;
}

template<typename T>
b8
Queue<T>::pop() {
    if (is_empty()) {
        return false;
    }
    if (start == finish) {
        // One item case
        start = finish = cap;
    }
    else if (start == cap - 1) {
        // start is at the finish of the array,
        // so we reset to the front of the array
        start = 0;
    }
    else {
        // Normal case: just advance start
        ++start;
    }
    
    return true;
}

// NOTE(Momo): Not part of standard 'Queue' API, but in case you want to get
// someone from the Queue
template<typename T>
T*
Queue<T>::get(u32 index) {
    if (is_empty()) {
        return nullptr;
    }
    if (start <= finish) {
        if (index < start || index > finish) {
            return nullptr;
        }
        else {
            return data + index;
        }
    }
    else {
        if (index <= finish || (index >= start && index < cap)) {
            return data + index;
        }
        else {
            return nullptr;
        }
    }
}

template<typename T>
template<typename Callback, typename... Args>
void
Queue<T>::_foreach_sub(u32 start_in, u32 finish_in, Callback callback, Args... args) {
    for (u32 i = start_in; i <= finish_in; ++i) {
        T* item = data + i;
        callback(item, args...);
    }
}

template<typename T>
template<typename Callback, typename... Args>
void
Queue<T>::foreach(Callback callback, Args... args) {
    if (is_empty()) {
        return;
    }
    
    // Then update the living ones
    if (start <= finish) {
        _foreach_sub(start, finish, callback, args...);
    }
    else {
        _foreach_sub(start, cap - 1, callback, args...);
        _foreach_sub(0, finish, callback, args...);
    }
    
}

template<typename T>
template<typename Callback, typename... Args>
void
Queue<T>::pop_until(Callback callback, Args... args) {
    T* item = this->next();
    while(item != nullptr && callback(item, args...)) {
        this->pop();
        item = this->next();
    }
}

