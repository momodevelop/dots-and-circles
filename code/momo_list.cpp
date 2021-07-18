
template<typename T>
inline T& 
List<T>::operator[](u32 index) {
    ASSERT(index < count); 
    return data[index];
}


template<typename T>
static inline b8
List_Init(List<T>* l, T* data, u32 cap) {
    if (!data || cap == 0) {
        return false;
    }
    l->data = data;
    l->cap = cap;
    l->count = 0;
    
    return true;
}

template<typename T>
static inline b8
List_Alloc(List<T>* l, Arena* arena, u32 cap) {
    T* buffer = Arena_Push_Array<T>(arena, cap);
    if (!buffer) {
        return false;
    }
    return List_Init(l, buffer, cap);
}



template<typename T>
static inline void
List_Clear(List<T>* l) {
    l->count = 0;
}

// NOTE(Momo): Push, but does not override whatever's in
// the container because user might have already initialized
// something important before that remained inside
template<typename T>
static inline T*
List_Push(List<T>* l) {
    if(l->count < l->cap) {
        T* ret = l->data + l->count++;
        return ret;
    }
    return nullptr;
}

template<typename T>
static inline T*
List_Push_Item(List<T>* l, T item) {
    if(l->count < l->cap) {
        T* ret = l->data + l->count++;
        (*ret) = item;
        return ret;
    }
    return nullptr;
}

// NOTE(Momo): "Swap last element and remove"
template<typename T>
static inline b8
List_Slear(List<T>* l, u32 index) {
    if (index < l->count) {
        l->data[index] = l->data[l->count-1];
        --l->count;
        return true;
    }
    else {
        return false;
    }
}

template<typename T>
static inline b8
List_Pop(List<T>* l) {
    if (l->count != 0) {
        --l->count;
        return true;
    }
    return false;
}

template<typename T>
static inline T*
List_Last(List<T>* l) {
    if (l->count == 0){
        return nullptr;
    }
    else {
        return List_Get(l, l->count - 1);
    }
    
}

template<typename T>
static inline u32
List_Remaining(List<T>* l) {
    return l->cap - l->count;
}



template<typename T>
static inline T*
List_Get(List<T>* l, u32 index) {
    if(index < l->count) {
        return l->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
static inline T* 
operator+(List<T> l, u32 index) {
    return List_Get(&l, index);
}

// TODO: refactor this into the class?
template<typename T, typename Callback, typename... Args>
static inline void
List_Foreach_Slear_If(List<T>* l, Callback callback, Args... args) {
    for (u32 i = 0; i < l->count;) {
        if (callback(l->data + i, args...)) {
            List_Slear(l, i);
            continue;
        }
        ++i;
    }
}

template<typename T, typename Callback, typename... Args>
static inline void
List_Foreach(List<T>* l, Callback callback, Args... args) {
    for (u32 i = 0; i < l->count; ++i) {
        callback(l->data + i, args...);
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
    u32 actual_index = this->list->count - this->index - 1;
    return this->list[actual_index];
}

