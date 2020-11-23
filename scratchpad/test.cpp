#include "../code/mm_core.h"
#include "../code/mm_string.h"
#include "../code/mm_list.h"

#include <iostream>
#include <vector>
using namespace std;

namespace m44f {
    struct ctx {
        u32 I;
    };

    static inline void Print() {
        cout << "Hello World" << endl;
    }
}

int main() {
    m44f M;
    Print();

}
