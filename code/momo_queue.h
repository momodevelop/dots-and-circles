/* date = May 22nd 2021 2:37 pm */

#ifndef MOMO_QUEUE_H
#define MOMO_QUEUE_H


//~ NOTE(Momo): Queue

template<typename type>
struct queue {
    type* Data;
    u32 Cap;
    u32 Begin;
    u32 End;
};


template<typename type>
static inline b8
Queue_Init(queue<type>* Q, type* Buffer, u32 BufferCap) {
    if (!Buffer || BufferCap < 0) {
        return false;
    }
    Q->Begin = Q->End = BufferCap;
    Q->Cap = BufferCap;
    Q->Data = Buffer;
    
    return true;
}

template<typename type>
static inline b8
Queue_New(queue<type>* Q, arena* Arena, u32 Cap) {
    type* Buffer = Arena_PushArray<type>(Arena, Cap);
    if (!Buffer) {
        return false;
    }
    
    return Queue_Init(Q, Buffer, Cap);
}

template<typename type>
static inline b8 
Queue_IsEmpty(queue<type>* Q) {
    return Q->Begin == Q->Cap || 
        Q->End == Q->Cap;
}

template<typename type>
static inline b8
Queue_IsFull(queue<type>* Q) {
    b8 NormalCase = (Q->Begin == 0 && Q->End == Q->Cap-1);
    b8 BackwardCase = Q->End == (Q->Begin-1);
    
    return !Queue_IsEmpty(Q) && (NormalCase || BackwardCase);
}

template<typename type>
static inline type* 
Queue_Push(queue<type>* Q, type InitItem = {}) {
    if (Queue_IsFull(Q)) {
        return nullptr;
    }
    else if (Queue_IsEmpty(Q)) {
        Q->Begin = Q->End = 0;
    }
    else if (Q->End == Q->Cap - 1) {
        // End is already at the back of the array
        Q->End = 0;
    }
    else {
        // Normal case: just advance End
        ++Q->End;
    }
    return Q->Data + Q->End;
}

template<typename type>
static inline type*
Queue_Next(queue<type>* Q) {
    if (Queue_IsEmpty(Q)) {
        return nullptr;
    }
    return Q->Data + Q->Begin;
}

template<typename type>
static inline b8
Queue_Pop(queue<type>* Q) {
    if (Queue_IsEmpty(Q)) {
        return false;
    }
    if (Q->Begin == Q->End) {
        // One item case
        Q->Begin = Q->End = Q->Cap;
    }
    else if (Q->Begin == Q->Cap - 1) {
        // Begin is at the end of the array,
        // so we reset to the front of the array
        Q->Begin = 0;
    }
    else {
        // Normal case: just advance Begin
        ++Q->Begin;
    }
    
    return true;
}

// NOTE(Momo): Not part of standard 'Queue' API, but in case you want to get
// someone from the queue
template<typename type>
static inline type*
Queue_Get(queue<type>* Q, u32 Index) {
    if (Queue_IsEmpty(Q)) {
        return nullptr;
    }
    if (Q->Begin <= Q->End) {
        if (Index < Q->Begin || Index > Q->End) {
            return nullptr;
        }
        else {
            return Q->Data + Index;
        }
    }
    else {
        if (Index <= Q->End || (Index >= Q->Begin && Index < Q->Cap)) {
            return Q->Data + Index;
        }
        else {
            return nullptr;
        }
    }
}

template<typename type, typename callback, typename... args>
static inline void
Queue_ForEachSub(queue<type>* Q, u32 Begin, u32 End, callback Callback, args... Args) {
    for (u32 I = Begin; I <= End; ++I) {
        type* Item = Q->Data + I;
        Callback(Item, Args...);
    }
}

template<typename type, typename callback, typename... args>
static inline void
Queue_ForEach(queue<type>* Q, callback Callback, args... Args) {
    if (Queue_IsEmpty(Q)) {
        return;
    }
    
    // Then update the living ones
    if (Q->Begin <= Q->End) {
        Queue_ForEachSub(Q, Q->Begin, Q->End, Callback, Args...);
    }
    else {
        Queue_ForEachSub(Q, Q->Begin, Q->Cap - 1, Callback, Args...);
        Queue_ForEachSub(Q, 0, Q->End, Callback, Args...);
    }
    
}

template<typename type, typename callback, typename... args>
static inline void
Queue_PopUntil(queue<type>* Q, callback Callback, args... Args) {
    particle * P = Queue_Next(Q);
    while(P != nullptr && Callback(P, Args...)) {
        Queue_Pop(Q);
        P = Queue_Next(Q);
    }
}


#endif //MOMO_QUEUE_H
