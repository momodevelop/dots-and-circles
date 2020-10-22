#ifndef MOMO_DYNAMIC_BUFFER_H
#define MOMO_DYNAMIC_BUFFER_H

#include "mm_std.h"

#define mmdbf_Capacity(a) ((a) ? mmdbf__RawCapacity(a) : 0)
#define mmdbf_Count(a) ((a) ? mmdbf__RawCount(a) : 0)
#define mmdbf_Push(a, o) (mmdbf__TryGrow(a,1), (a)[mmdbf__RawCount(a)++] = (o))
#define mmdbf_Free(a) ((a) ? free(mmdbf__Raw(a)),0 : 0)
#define mmdbf_Last(a) ((a)[mmdbf_Count(a)-1])
#define mmdbf_Add(a,n) (mmdbf__TryGrow(a,n), mmdbf__RawCount(a)+=(n))
#define mmdbf_Pop(a) (mmdbf__RawCount(a)-=1)

#define mmdbf__Raw(a) ((u32*)(a) - 2)
#define mmdbf__RawCapacity(a) mmdbf__Raw(a)[0]
#define mmdbf__RawCount(a) mmdbf__Raw(a)[1]
#define mmdbf__IsNeedGrow(a,n) ((a) == nullptr || mmdbf__RawCount(a) + (n) >= mmdbf__RawCapacity(a))
#define mmdbf__TryGrow(a,n) (mmdbf__IsNeedGrow(a,n) ? mmdbf__Grow(a,n) : 0)
#define mmdbf__Grow(a,n) (*((void **)&(a)) = mmdbf__GrowSub((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void* mmdbf__GrowSub(void * Arr, u32 Increment, u32 ObjSize) {
    u32 DoubleCapacity = mmdbf_Capacity(Arr) * 2;
    u32 RequiredCapacity = mmdbf_Count(Arr) + Increment;
    u32 NewCapacity = Maximum(RequiredCapacity, DoubleCapacity);
    
    u32 *Ret = (u32*)realloc((Arr ? mmdbf__Raw(Arr) : 0), 
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
