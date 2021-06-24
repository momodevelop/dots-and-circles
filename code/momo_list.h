/* date = May 22nd 2021 3:34 pm */

#ifndef MOMO_LIST_H
#define MOMO_LIST_H
//~ NOTE(Momo): list
template<typename type>
struct list {
    type* Data;
    u32 Count;
    u32 Cap;
    
    auto& operator[](u32 Index) {
        Assert(Index < Count); 
        return Data[Index];
    }
};

struct list_iterator {
    u32 Index;
};


template<typename type>
static inline b8
List_Init(list<type>* L, type* Data, u32 Cap) {
    if (!Data || Cap == 0) {
        return false;
    }
    L->Data = Data;
    L->Cap = Cap;
    L->Count = 0;
    
    return true;
}

template<typename type>
static inline b8
List_New(list<type>* L, Arena* arena, u32 Cap) {
    type* Buffer = arena->push_array<type>(Cap);
    if (!Buffer) {
        return false;
    }
    return List_Init(L, Buffer, Cap);
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
    return nullptr;
}

template<typename type>
static inline type*
List_PushItem(list<type>* L, type Item) {
    if(L->Count < L->Cap) {
        type* Ret = L->Data + L->Count++;
        (*Ret) = Item;
        return Ret;
    }
    return nullptr;
}

// NOTE(Momo): "Swap last element and remove"
template<typename type>
static inline b8
List_Slear(list<type>* L, u32 Index) {
    if (Index < L->Count) {
        L->Data[Index] = L->Data[L->Count-1];
        --L->Count;
        return true;
    }
    else {
        return false;
    }
}

template<typename type>
static inline b8
List_Pop(list<type>* L) {
    if (L->Count != 0) {
        --L->Count;
        return true;
    }
    return false;
}

template<typename type>
static inline type*
List_Last(list<type>* L) {
    if (L->Count == 0){
        return nullptr;
    }
    else {
        return List_Get(L, L->Count - 1);
    }
    
}

template<typename type>
static inline u32
List_Remaining(list<type>* L) {
    return L->Cap - L->Count;
}



template<typename type>
static inline type*
List_Get(list<type>* L, u32 Index) {
    if(Index < L->Count) {
        return L->Data + Index;
    }
    else {
        return nullptr;
    }
}

template<typename type>
static inline type* 
operator+(list<type> L, u32 Index) {
    return List_Get(&L, Index);
}

template<typename type, typename callback, typename... args>
static inline void
List_ForEachSlearIf(list<type>* L, callback Callback, args... Args) {
    for (u32 I = 0; I < L->Count;) {
        type* Item = List_Get(L, I);
        if (Callback(Item, Args...)) {
            List_Slear(L, I);
            continue;
        }
        ++I;
    }
}

template<typename type, typename callback, typename... args>
static inline void
List_ForEach(list<type>* L, callback Callback, args... Args) {
    for (u32 I = 0; I < L->Count; ++I) {
        type* Item = List_Get(L, I);
        Callback(Item, Args...);
    }
}

//~ NOTE(Momo): Forward Iterator
template<typename type>
struct list_forward_itr {
    list<type>* L;
    u32 Index;
    
    type& operator->() {
        return L->Data[Index];
    }
    
    
};


template<typename type>
static inline list_forward_itr<type>
List_ForwardItr_Begin(list<type>* L) {
    return { *L, 0 };
}

template<typename type>
static inline list_forward_itr<type>
List_ForwardItr_End(list<type>* L) {
    return { *L, L->Count - 1 };
}

template<typename type>
static inline b8
operator!=(list_forward_itr<type> Lhs, list_forward_itr<type> Rhs) {
    return Lhs.Index != Rhs.Index;
}

template<typename type>
static inline list_forward_itr<type>&
operator++(list_forward_itr<type>& Itr) {
    ++Itr.Index;
    return Itr;
}

template<typename type>
static inline type&
operator*(list_forward_itr<type>& Itr) {
    return Itr.List->Data[Itr.Index];
}
//~ NOTE(Momo): Reverse Iterator
template<typename type>
struct list_reverse_itr {
    list<type>* L;
    u32 Index;
    
    type& operator->() {
        return L->Data[Index];
    }
    
    
};


template<typename type>
static inline list_reverse_itr<type>
List_ReverseItr_Begin(list<type>* L) {
    return { *L, 0 };
}

template<typename type>
static inline list_reverse_itr<type>
List_ReverseItr_End(list<type>* L) {
    return { *L, L->Count - 1 };
}

template<typename type>
static inline b8
operator!=(list_reverse_itr<type> Lhs, list_reverse_itr<type> Rhs) {
    return Lhs.Index != Rhs.Index;
}

template<typename type>
static inline list_reverse_itr<type>&
operator++(list_reverse_itr<type>& Itr) {
    ++Itr.Index;
    return Itr;
}

template<typename type>
static inline type&
operator*(list_reverse_itr<type>& Itr) {
    u32 ActualIndex = Itr.List->Count - Itr.Index - 1;
    return Itr.List->Data[ActualIndex];
}


#endif //MOMO_LIST_H
