#include <stdio.h>
#include "../../code/momo.h"

int main() {
    u32 A[128];
    list<u32> _L = {};
    list<u32>* L = &_L;
    
    List_Init(L, A, 128);
    
    for (u32 I = 0; I < 10; ++I) 
        List_PushItem(L, I);
    
    
    for(auto&& E : L) {
        printf("%d", E);
    }
    printf("\n");
    
    
    
    
}