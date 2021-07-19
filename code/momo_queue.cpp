
template<typename T>
static inline b8
Queue_Init(Queue<T>* q, T* buffer, u32 cap) {
    if (!buffer || cap < 0) {
        return false;
    }
    q->start = q->finish = cap;
    q->cap = cap;
    q->data = buffer;
    
    return true;
}

template<typename T>
static inline b8
Queue_Alloc(Queue<T>* q, Arena* arena, u32 cap) {
    T* buffer = Arena_PushArray<T>(arena, cap);
    if (!buffer) {
        return false;
    }
    
    return Queue_Init(q, buffer, cap);
}

template<typename T>
static inline b8 
Queue_IsEmpty(Queue<T>* q) {
    return q->start == q->cap || 
        q->finish == q->cap;
}

template<typename T>
static inline b8
Queue_IsFull(Queue<T>* q) {
    b8 normal_case = (q->start == 0 && q->finish == q->cap-1);
    b8 backward_case = q->finish == (q->start-1);
    
    return !Queue_IsEmpty(q) && (normal_case || backward_case);
}

template<typename T>
static inline T* 
Queue_Push(Queue<T>* q) {
    if (Queue_IsFull(q)) {
        return nullptr;
    }
    else if (Queue_IsEmpty(q)) {
        q->start = q->finish = 0;
    }
    else if (q->finish == q->cap - 1) {
        // finish is already at the back of the array
        q->finish = 0;
    }
    else {
        // Normal case: just advance finish
        ++q->finish;
    }
    return q->data + q->finish;
    
}

template<typename T>
static inline T*
Queue_Next(Queue<T>* q) {
    if (Queue_IsEmpty(q)) {
        return nullptr;
    }
    return q->data + q->start;
}

template<typename T>
static inline b8
Queue_Pop(Queue<T>* q) {
    if (Queue_IsEmpty(q)) {
        return false;
    }
    if (q->start == q->finish) {
        // One item case
        q->start = q->finish = q->cap;
    }
    else if (q->start == q->cap - 1) {
        // start is at the finish of the array,
        // so we reset to the front of the array
        q->start = 0;
    }
    else {
        // Normal case: just advance start
        ++q->start;
    }
    
    return true;
}

// NOTE(Momo): Not part of standard 'Queue' API, but in case you want to get
// someone from the Queue
template<typename T>
static inline T*
Queue_Get(Queue<T>* q, u32 index) {
    if (Queue_IsEmpty(q)) {
        return nullptr;
    }
    if (q->start <= q->finish) {
        if (index < q->start || index > q->finish) {
            return nullptr;
        }
        else {
            return q->data + index;
        }
    }
    else {
        if (index <= q->finish || (index >= q->start && index < q->cap)) {
            return q->data + index;
        }
        else {
            return nullptr;
        }
    }
}


template<typename T, typename Callback, typename... Args>
static inline void
_Queue_ForEach(Queue<T>* q, u32 start, u32 finish, Callback callback, Args... args) {
    for (u32 i = q->start; i <= q->finish; ++i) {
        T* item = q->data + i;
        callback(item, args...);
    }
}

template<typename T, typename Callback, typename... Args>
static inline void
Queue_ForEach(Queue<T>* q, Callback callback, Args... args) {
    if (Queue_IsEmpty(q)) {
        return;
    }
    
    // Then update the living ones
    if (q->start <= q->finish) {
        _Queue_ForEach(q, q->start, q->finish, callback, args...);
    }
    else {
        _Queue_ForEach(q, q->start, q->cap - 1, callback, args...);
        _Queue_ForEach(q, 0, q->finish, callback, args...);
    }
    
}

template<typename T, typename Callback, typename... Args>
static inline void
Queue_PopUntil(Queue<T>* q, Callback callback, Args... args) {
    T* item = Queue_Next(q);
    while(item != nullptr && callback(item, args...)) {
        Queue_Pop(q);
        item = Queue_Next(q);
    }
}

