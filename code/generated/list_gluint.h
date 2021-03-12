// Generated by Codegen
#ifndef LIST_GLUINT
#define LIST_GLUINT
// list_gluint = list name
// array_gluint = array name
// GLuint = type name
// ListGLuint_ = function prefix



// NOTE(Momo): Requires you to #include the definition of array_gluint before this
struct list_gluint : array_gluint {
    u32 Capacity; // Total number of borrowable objects.
};

// Constructors
static inline list_gluint
ListGLuint_CreateFromMemory(ListGLuint_* Arr, u32 Capacity) {
    list_gluint Ret = {};
    Ret.E = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

static inline list_gluint
ListGLuint_CreateFromArena(arena* Arena, u32 Capacity) {
    list_gluint Ret = {};
    Ret.E = Arena_PushArray(ListGLuint_, Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}


static inline void 
ListGLuint_Clear(list_gluint* List) {
    List->Count = 0;
}

static inline void
ListGLuint_Copy(list_gluint* Dest, array_gluint* Src) {
    Assert(Src->Count <= Dest->Capacity);
    for (u32 I = 0; I < Src->Count; ++I ) {
        Dest->E[I] = Src->E[I];
    }
    Dest->Count = Src->Count;
}


static inline u32
ListGLuint_Remaining(list_gluint* List) {
    return List->Capacity - List->Count;
}

static inline b32
ListGLuint_Push(list_gluint* List, ListGLuint_ Obj) {
    if(List->Count >= List->Capacity) {
		return false;
	}
    List->E[List->Count++] = Obj;
	return true;
}

static inline b32
ListGLuint_PushArray(list_gluint* Dest, array_gluint Src) {
    if(Dest->Count + Src.Count > Dest->Capacity) {
		return false;
	}
    for ( u32 i = 0; i < Src.Count; ++i ) {
        Dest->E[Dest->Count++] = Src.E[i];
    }
	return true;
}


static inline void
ListGLuint_Pop(list_gluint* List) {
    Assert(List->Count > 0);
    --List->Count;
}

static inline ListGLuint_*
ListGLuint_Front(list_gluint* List) {
    Assert(List->Count > 0);
    return List->E;
}

static inline ListGLuint_*
ListGLuint_Back(list_gluint* List) {
    Assert(List->Count > 0);
    return List->E + List->Count - 1;
}


static inline void
ListGLuint_Remove(list_gluint* List, u32 Index) {
    Assert(Index < List->Count);
    for (; Index < List->Count - 1; ++Index) {
        List->E[Index] = List->E[Index + 1];
    }
    --List->Count;
}

// Use this to remove objects if order does not matter!
static inline void
ListGLuint_SwapRemove(list_gluint* List, u32 Index) {
    //(*Obj) = List->E[List->Count - 1];
    List->E[Index] = List->E[List->Count - 1];
    --List->Count;
}



/*
static inline usize
ListGLuint_RemoveIf(list_gluint* List, unary_comparer UnaryComparer) {
    usize Index = Find(List, UnaryComparer);
    Remove(List, Index);
    return Index;
}

*/
#endif