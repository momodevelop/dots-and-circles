#include "../code/ryoji.h"
#include <stdio.h>

// NOTE(Momo): https://gist.github.com/olibre/3d0774df0f7a16e2da10fae2b2f26c4f

template<class T>
struct base {
    void interface() {
        ((T*)(this))->implementation();
    }
    
    private:
    
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