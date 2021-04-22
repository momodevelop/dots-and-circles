/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): array
template<typename type>
struct array {
    type* Data;
    u32 Count;
};

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
static inline type*
List_Last(list<type>* L) {
    if (L->Count == 0){
        return Null;
    }
    else {
        return L->Data + L->Count;
    }
    
}

template<typename type>
static inline u32
List_Remaining(list<type>* L) {
    return L->Cap - L->Count;
}

#endif //MM_ARRAY_H
