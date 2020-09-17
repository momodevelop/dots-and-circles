#ifndef RYOJI_DYNAMIC_BUFFER_H
#define RYOJI_DYNAMIC_BUFFER_H

#include "ryoji.h"

#define DynBufferCapacity(a) ((a) ? _DynBufferRawCapacity(a) : 0)
#define DynBufferCount(a) ((a) ? _DynBufferRawCount(a) : 0)
#define DynBufferPush(a, o) (_DynBufferTryGrow(a,1), (a)[_DynBufferRawCount(a)++] = (o))
#define DynBufferFree(a) ((a) ? free(_DynBufferRaw(a)),0 : 0)
#define DynBufferLast(a) ((a)[DynBufferCount(a)-1])
#define DynBufferAdd(a,n) (_DynBufferTryGrow(a,n), _DynBufferRawCount(a)+=(n))
#define DynBufferPop(a) (_DynBufferRawCount(a)-=1)

#define _DynBufferRaw(a) ((u32*)(a) - 2)
#define _DynBufferRawCapacity(a) _DynBufferRaw(a)[0]
#define _DynBufferRawCount(a) _DynBufferRaw(a)[1]
#define _DynBufferIsNeedGrow(a,n) ((a) == nullptr || _DynBufferRawCount(a) + (n) >= _DynBufferRawCapacity(a))
#define _DynBufferTryGrow(a,n) (_DynBufferIsNeedGrow(a,n) ? _DynBufferGrow(a,n) : 0)
#define _DynBufferGrow(a,n) (*((void **)&(a)) = _DynBufferGrowSub((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void* _DynBufferGrowSub(void * Arr, u32 Increment, u32 ObjSize) {
    u32 DoubleCapacity = DynBufferCapacity(Arr) * 2;
    u32 RequiredCapacity = DynBufferCount(Arr) + Increment;
    u32 NewCapacity = Maximum(RequiredCapacity, DoubleCapacity);
    
    u32 *Ret = (u32*)realloc((Arr ? _DynBufferRaw(Arr) : 0), 
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
