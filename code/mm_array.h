/* date = April 20th 2021 10:57 am */

#ifndef MM_ARRAY_H
#define MM_ARRAY_H

template<typename type>
struct array {
    type* Data;
    u32 Count;
    
    auto& operator[](u32 Index) {
        Assert(Index < Count);
        return this->Data[Index];
    }
};

template<typename type>
static inline array<type>
Array_Create(type* Data, u32 Size) {
    array<type> Ret = {};
    Ret.Data = Data;
    Ret.Size = Size;
    
    return Ret;
}

template<typename type>
static inline array<type>
Array_FromArena(arena* Arena, u32 Size) {
    type* Data = Arena_PushArray(type, Arena, Size);
    return Array_Create<type>(Data, Size);
}

template<typename type>
struct list : array<type> {
    u32 Cap;
};

template<typename type>
static inline list<type>
List_Create(type* Data, u32 Capacity) {
    list<type> Ret = {};
    Ret.Data = Data;
    Ret.Cap = Capacity;
    
    return Ret;
}

template<typename type>
static inline list<type>
List_FromArena(arena* Arena, u32 Capacity) {
    type* Data = Arena_PushArray(type, Arena, Capacity);
    return List_Create<type>(Data, Capacity);
}


template<typename type>
static inline void
List_Push(list<type>* L, type Item) {
    L->Data[L->Count++] = Item;
}

template<typename type>
static inline u32
List_Remaining(list<type>* L) {
    return L->Cap - L->Count;
}

#endif //MM_ARRAY_H
