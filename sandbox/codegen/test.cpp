#include <stdio.h>
#include <stdlib.h>

struct test{
    
};

template<typename T>
static inline void
Boo() {
    test Ret; 
}

template<typename T>
static inline test
Foo() {
    Boo<T>();
    test Ret = {};
    return Ret;
}

int main() {
    Foo<int>();


}
