/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

//~ NOTE(Momo): General array struct
template<typename type>
struct array {
    type* Data;
    u32 Count;
};

template<typename type>
struct list {
    union {
        array<type> Arr;
        struct {
            type* Data;
            u32 Count;
        };
    };
};

//~ NOTE(Momo): Array of fixed size
template<typename type, u32 Cap>
struct farray {
    type Data[Cap];
    
    inline type& operator*() {
        return Data[0];
    }
    
    inline type* operator+(u32 Index) {
        Assert(Index < Cap);
        return Data + Index;
    }
    
    auto& operator[](u32 Index) {
        Assert(Index < Cap);
        return Data[Index];
    }
};




//~ NOTE(Momo): Short for fixed-array list
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
static inline type*
FList_Push(flist<type,Cap>* L, type Item) {
    if(L->Count < Cap) {
        type* Ret = L->Data + L->Count++;
        (*Ret) = Item;
        return Ret;
    }
    return Null;
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
        return L->Data + Index;
    }
    else {
        return Null;
    }
}

template<typename type, u32 Cap>
static inline type*
FList_Last(flist<type, Cap>* L) {
    if (L->Count == 0){
        return Null;
    }
    else {
        return L->Data + L->Count;
    }
    
}

template<typename type, u32 Cap>
static inline u32
FList_Remaining(flist<type,Cap>* L) {
    return Cap - L->Count;
}


#endif //MM_ARRAY_H
