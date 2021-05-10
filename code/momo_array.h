/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename type>
struct array {
    type* Data;
    u32 Count;
    
    auto& operator[](u32 Index) {
        Assert(Index < Count); 
        return Data[Index];
    }
};

template<typename type>
static inline b32
Array_InitFromArena(array<type>* L, arena* Arena, u32 Count) {
    type* Buffer = Arena_PushArray(type, Arena, Count);
    if (!Buffer) {
        return False;
    }
    L->Data = Buffer;
    L->Count = Count;
    return True;
}

template<typename type>
static inline type*
Array_Get(array<type>* L, u32 Index) {
    if(Index < L->Count) {
        return L->Data + Index;
    }
    else {
        return Null;
    }
}

template<typename type>
static inline type* 
operator+(array<type> L, u32 Index) {
    return Array_Get(&L, Index);
}

template<typename type, typename pred>
static inline void
Array_ForEach(array<type>* L, pred Pred) {
    for (u32 I = 0; I < L->Count; ++I) {
        Pred(L->Data + I);
    }
}

//~ NOTE(Momo): list
template<typename type>
struct list : array<type> {
    u32 Cap;
};



template<typename type>
static inline void
List_Init(list<type>* L, type* Data, u32 Cap) {
    L->Data = Data;
    L->Cap = Cap;
    L->Count = 0;
}

template<typename type>
static inline b32
List_InitFromArena(list<type>* L, arena* Arena, u32 Cap) {
    type* Buffer = Arena_PushArray(type, Arena, Cap);
    if (!Buffer) {
        return False;
    }
    L->Data = Buffer;
    L->Cap = Cap;
    L->Count = 0;
    return True;
}



template<typename type>
static inline void 
List_Clear(list<type>* L) {
    L->Count = 0;
}

// NOTE(Momo): Push, but does not override whatever's in
// the container because user might have already initialized
// something important before that remained inside
template<typename type>
static inline type*
List_Push(list<type>* L) {
    if(L->Count < L->Cap) {
        type* Ret = L->Data + L->Count++;
        return Ret;
    }
    return Null;
}

template<typename type>
static inline type*
List_PushItem(list<type>* L, type Item) {
    if(L->Count < L->Cap) {
        type* Ret = L->Data + L->Count++;
        (*Ret) = Item;
        return Ret;
    }
    return Null;
}

// NOTE(Momo): "Swap last element and remove"
template<typename type>
static inline b32
List_Slear(list<type>* L, u32 Index) {
    if (Index < L->Count) {
        L->Data[Index] = L->Data[L->Count-1];
        --L->Count;
        return True;
    }
    else {
        return False;
    }
}

template<typename type>
static inline b32
List_Pop(list<type>* L) {
    if (L->Count != 0) {
        --L->Count;
        return True;
    }
    return False;
}

template<typename type>
static inline type*
List_Last(list<type>* L) {
    if (L->Count == 0){
        return Null;
    }
    else {
        return L->Data + L->Count - 1;
    }
    
}

template<typename type>
static inline u32
List_Remaining(list<type>* L) {
    return L->Cap - L->Count;
}

template<typename type, typename pred>
static inline void
List_ForEach(list<type>* L, pred Pred) {
    for (u32 I = 0; I < L->Count; ++I) {
        Pred(L->Data + I);
    }
}

//~ NOTE(Momo): Queue

template<typename type>
struct queue {
    type* Data;
    u32 Count;
    u32 Begin;
    u32 End;
};

template<typename type>
static inline void
Queue_Init(queue<type>* Q, type* Buffer, u32 BufferCount) {
    Q->Begin = Q->End = BufferCount;
    Q->Count = BufferCount;
    Q->Data = Buffer;
    
}

template<typename type>
static inline b32
Queue_InitFromArena(queue<type>* Q, arena* Arena, u32 Count) {
    type* Buffer = Arena_PushArray(type, Arena, Count);
    if (!Buffer) {
        return False;
    }
    Q->Begin = Q->End = Count;
    Q->Count = Count;
    
    return True;
}

template<typename type>
static inline b32 
Queue_IsEmpty(queue<type>* Q) {
    return Q->Begin == Q->Count || 
        Q->End == Q->Count;
}

template<typename type>
static inline b32
Queue_IsFull(queue<type>* Q) {
    b32 NormalCase = (Q->Begin == 0 && Q->End == Q->Count-1);
    b32 BackwardCase = Q->End == (Q->Begin-1);
    
    return !Queue_IsEmpty(Q) && (NormalCase || BackwardCase);
}

template<typename type>
static inline b32 
Queue_PushItem(queue<type>* Q, type Item) {
    if (Queue_IsFull(Q)) {
        return False;
    }
    else if (Queue_IsEmpty(Q)) {
        Q->Begin = Q->End = 0;
    }
    else if (Q->End == Q->Count - 1) {
        // End is already at the back of the array
        Q->End = 0;
    }
    else {
        // Normal case: just advance End
        ++Q->End;
    }
    
    Q->Data[Q->End] = Item;
    
    return True;
}

template<typename type>
static inline type*
Queue_Next(queue<type>* Q) {
    if (Queue_IsEmpty(Q)) {
        return Null;
    }
    return Q->Data + Q->Begin;
}

template<typename type>
static inline b32
Queue_Pop(queue<type>* Q) {
    if (Queue_IsEmpty(Q)) {
        return False;
    }
    if (Q->Begin == Q->End) {
        // One item case
        Q->Begin = Q->End = Q->Count;
    }
    else if (Q->Begin == Q->Count - 1) {
        // Begin is at the end of the array,
        // so we reset to the front of the array
        Q->Begin = 0;
    }
    else {
        // Normal case: just advance Begin
        ++Q->Begin;
    }
    
    return True;
    
}

template<typename type, typename pred>
static inline void
Queue_ForEach(queue<type>* Q, pred Pred) {
    if (Queue_IsEmpty(Q)) {
        // Empty case
        return;
    }
    if (Q->Begin <= Q->End) {
        for (u32 I = Q->Begin; I <= Q->End; ++I) {
            Pred(Q->Data + I);
        }
    }
    else {
        for (u32 I = Q->Begin; I < Q->Count; ++I) {
            Pred(Q->Data + I);
        }
        
        for (u32 I = 0; I <= Q->End; ++I) {
            Pred(Q->Data + I);
        }
    }
}





#endif //MM_ARRAY_H
