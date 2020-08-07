#include <stdio.h>
#include <float.h>

union Foo {
    float F;
    char E[4];
};


int main() {
    Foo foo;
    
    
    foo.F = FLT_MAX;
    printf("%#X %#X %#X %#X", foo.E[0], foo.E[1], foo.E[2], foo.E[3] );
    
}