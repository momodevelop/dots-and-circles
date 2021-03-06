#include "../../code/mm_core.h"
#include "../../code/mm_sort.h"

#include <stdio.h>

static inline b32 
foo(int Lhs, int Rhs) {
    return Lhs > Rhs;
};


template<typename type, typename comparer>
static inline b32 
boo(comparer Comparer) {	
    return Comparer(1,2);
}

int main() {
    int Arr[] = { 9,7,5,3,1,2,4,6,8,0 };
    
    
    QuickSort<int>(Arr, ArrayCount(Arr), foo);
    
    for (int I = 0; I < ArrayCount(Arr); ++I) {
        printf("%d ", Arr[I]);
    }
    
    printf("\nHello World\n");
    printf("%d\n", boo<int>(foo));
}




