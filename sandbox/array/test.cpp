#include <stdio.h>
#include "../../code/momo.h"


#define Particle_Duration 3.0f
struct particle {
    u32 Index;
};


static inline void
Print(MM_Queue<particle>* Q) {
    auto Itr = Queue_Itr_Create(Q);
    while(Queue_Itr_IsValid(Q, Itr)) {
        particle* P = Queue_Itr_Get(Q, Itr);
        printf("%d", P->Index);
        Itr = Queue_Itr_Next(Q, Itr);
    }
}

int main() {
    MM_Queue<particle> Q;
    particle Particles[8];
    MM_Queue_Init(&Q, Particles, ArrayCount(Particles));
    printf("Testing empty\n");
    
    u32 I = 0;
    
    printf("Testing half full\n");
    Queue_PushItem(&Q, {I++}); 
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Print(&Q);
    printf("\n");
    //01234
    
    printf("Testing dequeue\n");
    MM_Queue_Pop(&Q);
    MM_Queue_Pop(&Q);
    Print(&Q);
    printf("\n");
    // 23
    
    printf("Testing MM_Queue until overshot\n");
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Print(&Q);
    printf("\n");
    // 23456789
    
    
    printf("Testing that overshoting won't work\n");
    Queue_PushItem(&Q, {0});
    Print(&Q);
    printf("\n");
    
    printf("Test removing all but one items\n");
    for(int i = 0; i < 7; ++i) {
        MM_Queue_Pop(&Q);
    }
    
}


