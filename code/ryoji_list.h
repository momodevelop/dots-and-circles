
#ifndef RYOJI_LIST_H
#define RYOJI_LIST_H

#include "ryoji.h"

#define ListCapacity(a) ((a) ? _ListRawCapacity(a) : 0)
#define ListCount(a) ((a) ? _ListRawCount(a) : 0)
#define ListPush(a, o) (_ListTryGrow(a,1), (a)[_ListRawCount(a)++] = (o))
#define ListFree(a) ((a) ? free(_ListRaw(a)),0 : 0)
#define ListLast(a) ((a)[ListCount(a)-1])
#define ListAdd(a,n) (_ListTryGrow(a,n), _ListRawCount(a)+=(n))
#define ListPop(a) (_ListRawCount(a)-=1)

#define _ListRaw(a) ((u32*)(a) - 2)
#define _ListRawCapacity(a) _ListRaw(a)[0]
#define _ListRawCount(a) _ListRaw(a)[1]
#define _ListIsNeedGrow(a,n) ((a) == nullptr || _ListRawCount(a) + (n) >= _ListRawCapacity(a))
#define _ListTryGrow(a,n) (_ListIsNeedGrow(a,n) ? _ListGrow(a,n) : 0)
#define _ListGrow(a,n) (*((void **)&(a)) = _ListGrowSub((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void* _ListGrowSub(void * Arr, u32 Increment, u32 ObjSize) {
    u32 DoubleCapacity = ListCapacity(Arr) * 2;
    u32 RequiredCapacity = ListCount(Arr) + Increment;
    u32 NewCapacity = Maximum(RequiredCapacity, DoubleCapacity);
    
    u32 *Ret = (u32*)realloc((Arr ? _ListRaw(Arr) : 0), 
                             (ObjSize * NewCapacity + sizeof(u32) * 2));
    if (Ret != nullptr) {
        if (Arr == nullptr) {
            Ret[1] = 0;
        }
        Ret[0] = NewCapacity;
    }
    
    
    return (void*)(Ret + 2);
    
}




#endif 
