
template<typename T>
T& 
List<T>::operator[](u32 index) {
    Assert(index < count); 
    return data[index];
}


template<typename T>
b8
List<T>::init(T* data_in, u32 cap_in) {
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
List<T>::alloc(Arena* arena, u32 cap_in) {
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
List<T>::slear(u32 index) {
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
u32
List<T>::remaining() {
    return cap - count;
}



template<typename T>
T*
List<T>::get(u32 index) {
    if(index < count) {
        return data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
T* 
List<T>::operator+(u32 index) {
    return get(index);
}

// TODO: refactor this into the class?
template<typename T, typename callback, typename... args>
static inline void
List_ForEachSlearIf(List<T>* L, callback Callback, args... Args) {
    for (u32 I = 0; I < L->count;) {
        T* Item = L->get(I);
        if (Callback(Item, Args...)) {
            L->slear(I);
            continue;
        }
        ++I;
    }
}

template<typename T>
template<typename Callback, typename... Args>
void
List<T>::foreach(Callback callback, Args... args) {
    for (u32 i = 0; i < count; ++i) {
        T* item = get(i);
        callback(item, args...);
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
operator!=(List_Forward_Itr<T> lhs, List_Forward_Itr<T> rhs) {
    return lhs.index != rhs.index;
}

template<typename T>
static inline List_Forward_Itr<T>&
operator++(List_Forward_Itr<T>& itr) {
    ++itr.index;
    return itr;
}

template<typename T>
static inline T&
operator*(List_Forward_Itr<T>& itr) {
    return itr.list[itr.index];
}
//~ NOTE(Momo): Reverse Iterator
template<typename T>
T& 
List_Reverse_Itr<T>::operator->(){
    return list->data[index];
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
static inline b8
operator!=(List_Reverse_Itr<T> Lhs, List_Reverse_Itr<T> Rhs) {
    return Lhs.Index != Rhs.Index;
}

template<typename T>
static inline List_Reverse_Itr<T>&
operator++(List_Reverse_Itr<T>& Itr) {
    ++Itr.Index;
    return Itr;
}


template<typename T>
T&
operator*(List_Reverse_Itr<T> itr) {
    u32 actual_index = itr.list->count - itr.index - 1;
    return itr.list[actual_index];
}

