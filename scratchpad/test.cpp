#include "../code/mm_core.h"

#include <iostream>
#include <vector>


struct test {
    u32 I;
};

int main() {
    using namespace std;
    char buffer[128];
    
    for ( i32 i = -128; i < 128; ++i) { 
        Itoa(buffer, i);
        cout << buffer << endl;
    }
    
}
