#include "../code/ryoji.h"
#include <stdio.h>


template<class T>
struct base {
    void interface() {
        ((T*)(this))->implementation();
    }
};

struct d1 {
    base<d1> Base;
    void implementation() {
        printf("Hello world\n");
    }
};

template<class T>
static inline void 
Foo(T* Boo) {
    Boo->Base.interface();
}

int main() {
    d1 Mode;
    Foo(&Mode);
    
    
}