#ifndef MOMO_FLEXI_BUFFER_H
#define MOMO_FLEXI_BUFFER_H

#include "mm_core.h"

#define FlexiCapacity(a) ((a) ? _FlexiRawCapacity(a) : 0)
#define FlexiCount(a) ((a) ? _FlexiRawCount(a) : 0)
#define FlexiPush(a, o) (_FlexiTryGrow(a,1), (a)[_FlexiRawCount(a)++] = (o))
#define FlexiFree(a) ((a) ? free(_FlexiRaw(a)),0 : 0)
#define FlexiLast(a) ((a)[FlexiCount(a)-1])
#define FlexiAdd(a,n) (_TryGrow(a,n), _FlexiRawCount(a)+=(n))
#define FlexiPop(a) (_FlexiRawCount(a)-=1)

#define _FlexiRaw(a) ((u32*)(a) - 2)
#define _FlexiRawCapacity(a) _FlexiRaw(a)[0]
#define _FlexiRawCount(a) _FlexiRaw(a)[1]
#define _FlexiIsNeedGrow(a,n) ((a) == nullptr || _FlexiRawCount(a) + (n) >= mmdbf__RawCapacity(a))
#define _FlexiTryGrow(a,n) (_FlexiIsNeedGrow(a,n) ? _FlexiGrow(a,n) : 0)
#define _FlexiGrow(a,n) (*((void **)&(a)) = _FlexiGrowSub((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void* _FlexiGrowSub(void * Arr, u32 Increment, u32 ObjSize) {
    u32 DoubleCapacity = FlexiCapacity(Arr) * 2;
    u32 RequiredCapacity = FlexiCount(Arr) + Increment;
    u32 NewCapacity = Maximum(RequiredCapacity, DoubleCapacity);
    
    u32 *Ret = (u32*)realloc((Arr ? _FlexiRaw(Arr) : 0), 
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
