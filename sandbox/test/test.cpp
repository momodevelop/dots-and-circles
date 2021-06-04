#include <stdio.h>
#include "../../code/momo.h"

int main() {
    u8 A[10];
    big_int _L = {};
    big_int* L = &_L;
    BigInt_Init(L, A, 100);
    _L = 1234567;
    
    u32 Rows = (u32)Sqrt((f32)L->Places); // Truncation expected
    u32 ExtraCharCount = L->Places - (Rows*Rows);
    printf("\n");
    printf("c: %d\n", L->Places);
    printf("r: %d\n", Rows);
    printf("x: %d\n", ExtraCharCount);
    for (u32 RowIndex = 0; RowIndex < Rows; ++RowIndex ) {
        u32 DistributedEvenly = ExtraCharCount/Rows;
        u32 Extra = ((ExtraCharCount%Rows) >= RowIndex + 1);
        printf("r%d: %d + %d + %d = %d\n", RowIndex, Rows, DistributedEvenly, Extra,
               Rows + DistributedEvenly + Extra );
    }
    printf("\n");
    
    fflush(0);
}