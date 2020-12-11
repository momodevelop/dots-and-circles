#ifndef MM_LINKED_LIST
#define MM_LINKED_LIST

#include "mm_core.h"

template<typename type>
struct dlink {
    dlink<type>* Prev;
    dlink<type>* Next;
    type Element;
};

template<typename type>
static inline dlink<type>
DLink(type Element) {
    dlink<type> Ret = {};
    Ret.Element = Element;
    return Ret;
}

template<typename type>
static inline void
Pushback(dlink<type>* Dest, dlink<type>* Node) {
    Node->Prev = Dest;
    Dest->Next = Element;
}

template<typename type>
struct dlink_list {
    dlink<type>* Head;
    dlink<type>* Tail;
    usize Count;

    // Careful, it's O(N)
    inline auto& operator[](usize Index) {
        Assert(Index < Count); 
        dlink<type>* Itr = Head;
        for (usize I = 0; I < Index; ++I) {
            Itr = Itr->Next;
        }
        return Itr->Element;
    }
        
};

template<typename type>
static inline void
PushBack(dlink_list<type>* Dest, dlink<type>* Element) {
    if (Dest->Head == nullptr) {
        Dest->Head = Element;
        Dest->Tail = Element;
        Element->Next = nullptr;
        Element->Prev = nullptr;
    }
    else {
        Dest->Tail->Next = Element;
        Element->Prev = Dest->Tail;
        Element->Next = nullptr;    
    }
    ++Dest->Count;
}




#endif


