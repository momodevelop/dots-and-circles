/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename T>
struct MM_Array {
    T* data;
    u32 count;
    
    auto& operator[](u32 index) {
        Assert(index < count); 
        return data[index];
    }
};

template<typename T>
static inline b8
MM_Array_InitFromArena(MM_Array<T>* a, MM_Arena* arena, u32 count) {
    T* buffer = MM_Arena_PushArray(T, arena, count);
    if (!buffer) {
        return false;
    }
    a->data = buffer;
    a->count = count;
    return true;
}

template<typename T>
static inline T*
MM_Array_Get(MM_Array<T>* a, u32 index) {
    if(index < a->count) {
        return a->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
static inline T* 
operator+(MM_Array<T> a, u32 index) {
    return MM_Array_Get(&a, index);
}

//~ NOTE(Momo): MM_List
template<typename T>
struct MM_List {
    T* data;
    u32 count;
    u32 cap;
    
    auto& operator[](u32 index) {
        Assert(index < count); 
        return data[index];
    }
};

template<typename T>
static inline b8
MM_List_InitFromArena(MM_List<T>* l, MM_Arena* arena, u32 cap) {
    T* buffer = MM_Arena_PushArray(T, arena, cap);
    if (!buffer) {
        return false;
    }
    l->data = buffer;
    l->cap = cap;
    l->count = 0;
    return true;
}



template<typename T>
static inline void 
MM_List_Clear(MM_List<T>* l) {
    l->count = 0;
}

// NOTE(Momo): Push, but does not override whatever's in
// the container because user might have already initialized
// something important before that remained inside
template<typename T>
static inline T*
MM_List_Push(MM_List<T>* l, T init_item = {}) {
    if(l->count < l->cap) {
        T* ret = l->data + l->count++;
        (*ret) = init_item;
        return ret;
    }
    return nullptr;
}

// NOTE(Momo): "Swap last element and remove"
template<typename T>
static inline b8
MM_List_Slear(MM_List<T>* l, u32 index) {
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
MM_List_Pop(MM_List<T>* l) {
    if (l->count != 0) {
        --l->count;
        return true;
    }
    return false;
}

template<typename T>
static inline T*
MM_List_Last(MM_List<T>* l) {
    if (l->count == 0){
        return Null;
    }
    else {
        return MM_List_Get(l, l->count - 1);
    }
    
}

template<typename T>
static inline u32
MM_List_Remaining(MM_List<T>* l) {
    return l->cap - l->count;
}

template<typename T>
static inline T*
MM_List_Get(MM_List<T>* l, u32 index) {
    if(index < l->count) {
        return l->data + index;
    }
    else {
        return nullptr;
    }
}

template<typename T>
static inline T* 
operator+(MM_List<T> l, u32 index) {
    return MM_List_Get(&l, index);
}

//~ NOTE(Momo): Queue
template<typename T>
struct MM_Queue {
    T* data;
    u32 count;
    u32 begin;
    u32 end;
};

template<typename T>
static inline b8
MM_Queue_Init(MM_Queue<T>* q, T* buffer, u32 buffer_count) {
    if (!buffer || buffer_count < 0) {
        return false;
    }
    q->begin = q->end = buffer_count;
    q->count = buffer_count;
    q->data = buffer;
    
    return true;
}

template<typename T>
static inline b8
MM_Queue_InitFromArena(MM_Queue<T>* q, MM_Arena* arena, u32 count) {
    T* buffer = MM_Arena_PushArray(T, arena, count);
    if (!buffer) {
        return false;
    }
    q->begin = q->end = count;
    q->count = count;
    
    return true;
}

template<typename T>
static inline b8 
MM_Queue_IsEmpty(MM_Queue<T>* q) {
    return q->begin == q->count || 
        q->end == q->count;
}

template<typename T>
static inline b8
MM_Queue_IsFull(MM_Queue<T>* q) {
    b8 normal_case = (q->begin == 0 && q->end == q->count-1);
    b8 backward_case = q->end == (q->begin-1);
    
    return !MM_Queue_IsEmpty(q) && (normal_case || backward_case);
}

template<typename T>
static inline T* 
MM_Queue_Push(MM_Queue<T>* q, T init_item = {}) {
    if (MM_Queue_IsFull(q)) {
        return nullptr;
    }
    else if (MM_Queue_IsEmpty(q)) {
        q->begin = q->end = 0;
    }
    else if (q->end == q->count - 1) {
        // end is already at the back of the array
        q->end = 0;
    }
    else {
        // Normal case: just advance end
        ++q->end;
    }
    return q->data + q->end;
}

template<typename T>
static inline T*
MM_Queue_Next(MM_Queue<T>* q) {
    if (MM_Queue_IsEmpty(q)) {
        return nullptr;
    }
    return q->data + q->begin;
}

template<typename T>
static inline b8
MM_Queue_Pop(MM_Queue<T>* q) {
    if (MM_Queue_IsEmpty(q)) {
        return false;
    }
    if (q->begin == q->end) {
        // One item case
        q->begin = q->end = q->count;
    }
    else if (q->begin == q->count - 1) {
        // begin is at the end of the array,
        // so we reset to the front of the array
        q->begin = 0;
    }
    else {
        // Normal case: just advance begin
        ++q->begin;
    }
    
    return true;
}

// NOTE(Momo): Not part of standard 'queue' API, but in case you want to get
// someone from the queue
template<typename T>
static inline T*
MM_Queue_Get(MM_Queue<T>* q, u32 index) {
    if (MM_Queue_IsEmpty(q)) {
        return nullptr;
    }
    if (q->begin <= q->end) {
        if (index < q->begin || index > q->end) {
            return nullptr;
        }
        else {
            return q->data + index;
        }
    }
    else {
        if (index <= q->end || (index >= q->begin && index < q->count)) {
            return q->data + index;
        }
        else {
            return nullptr;
        }
    }
}
#endif //MM_ARRAY_H
