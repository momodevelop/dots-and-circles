#include <iostream>

union Vec2f {
    float E[2];
    struct {
        float x , y;
    }
    
};

union Vec3f {
    // NOTE(Momo): 1
    float E[3]; 
    
    // NOTE(Momo): 2
    struct {
        float x,  y, z;
    };
    
    // NOTE(Momo): 3
    struct {
        float r, g, b;
    };
    
    // 
    struct {
        Vec2f xy;
        float _UNUSED;
    };
    
    // E[0] == x 
    struct {
        float _UNUSED;
        Vec2f yz;
    };
};






int main() {
    using namespace std;
    
    Foo foo;
    foo.c[0] = 1;
    foo.c[1] = 1;
    foo.c[2] = 1;
    foo.c[3] = 1;
    
    cout << foo.i << endl;
}