#include <stdio.h>
#include "../../code/momo.h"

int main() {
    printf("Hello World\n");
    u8 A[10];
    big_int _L = {};
    big_int* L = &_L;
    BigInt_Init(L, A, 100);
    BigInt_Add(L, 12345);
    
    _L += 12345;
    
    for(auto Beg = BigInt_ForwardItrBegin(L); Beg != BigInt_ForwardItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
    
    for(auto Beg = BigInt_ReverseItrBegin(L); Beg != BigInt_ReverseItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
    
    
    printf("Places: %d", L->Places);
    printf("\n");
    fflush(0);
}