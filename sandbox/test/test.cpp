#include <stdio.h>
#include "../../code/momo.h"

int main() {
    u8 A[10];
    big_int _L = {};
    big_int* L = &_L;
    BigInt_Init(L, A, 5);
    
    
    _L += 10;
    for(u32 I = 0; I < 99999999999; ++I) {
        _L += 1;
        for(auto Beg = BigInt_ReverseItrBegin(L); 
            Beg != BigInt_ReverseItrEnd(L); 
            ++Beg) 
        {
            printf("%d", (*Beg));
        }
        
        printf("\n");
    }
    
#if 0
    for(auto Beg = BigInt_ForwardItrBegin(L); Beg != BigInt_ForwardItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
    
    for(auto Beg = BigInt_ReverseItrBegin(L); Beg != BigInt_ReverseItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
#endif
    
    
    printf("Places: %d", L->Places);
    printf("\n");
    
}