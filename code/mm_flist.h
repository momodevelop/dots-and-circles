/* date = April 20th 2021 4:22 pm */
#ifndef MM_FLIST_H
#define MM_FLIST_H


// NOTE(Momo): Short for fixed-array list
template<typename type, u32 Cap>
struct flist {
    type Data[Cap];
    u32 Count;
};

template<typename type, u32 Cap>
static inline void 
FList_Init(flist<type, Cap>* L) {
    L->Count = 0;
}

template<typename type, u32 Cap>
static inline constexpr u32
FList_Capacity(flist<type,Cap> L) {
    return Cap;
}

template<typename type, u32 Cap>
static inline b32
FList_Push(flist<type,Cap>* L, type Item) {
    if(L->Count < Cap) {
        L->Data[L->Count++] = Item;
        return True;
    }
    return False;
}

// NOTE(Momo): "Swap last element and remove"
template<typename type, u32 Cap>
static inline b32
FList_Slear(flist<type,Cap>* L, u32 Index) {
    if (Index < L->Count) {
        L->Data[Index] = L->Data[L->Count-1];
        --L->Count;
        return True;
    }
    else {
        return False;
    }
}


template<typename type, u32 Cap>
static inline type*
FList_Get(flist<type, Cap>* L, u32 Index) {
    if(Index < L->Count) {
        return &L->Data[Index];
    }
    else {
        return Null;
    }
}

template<typename type, u32 Cap>
static inline u32
FList_Remaining(flist<type,Cap>* L) {
    return Cap - L->Count;
}


#endif //MM_FLIST_H
