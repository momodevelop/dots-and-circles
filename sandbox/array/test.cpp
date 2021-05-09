#include <stdio.h>
#include "../../code/momo.h"


#define Particle_Duration 3.0f
struct particle {
    u32 Index;
};


static inline void
PrintPred(particle* P) {
    printf("%d", P->Index);
}

int main() {
    queue<particle> Q;
    particle Particles[8];
    Queue_Init(&Q, Particles, ArrayCount(Particles));
    printf("Testing empty\n");
    Queue_ForEach(&Q, PrintPred);
    
    u32 I = 0;
    
    printf("Testing half full\n");
    Queue_PushItem(&Q, {I++}); 
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_ForEach(&Q, PrintPred);
    printf("\n");
    //01234
    
    printf("Testing dequeue\n");
    Queue_Pop(&Q);
    Queue_Pop(&Q);
    Queue_ForEach(&Q, PrintPred);
    printf("\n");
    // 23
    
    printf("Testing queue until overshot\n");
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_PushItem(&Q, {I++});
    Queue_ForEach(&Q, PrintPred);
    printf("\n");
    // 23456789
    
    
    printf("Testing that overshoting won't work\n");
    Queue_PushItem(&Q, {0});
    Queue_ForEach(&Q, PrintPred);
    printf("\n");
    
    printf("Test removing all but one items\n");
    for(int i = 0; i < 7; ++i) {
        Queue_Pop(&Q);
    }
    
    Queue_ForEach(&Q, PrintPred);
    
    
}


