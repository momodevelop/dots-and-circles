#include "../code/mm_core.h"
#include "../code/mm_string.h"
#include "../code/mm_list.h"

#include <iostream>
#include <vector>
using namespace std;

struct foo {
    u32 I;
};

static inline b32
Compare(const foo* gFoo) {
    return gFoo->I == 7;
}


int main() {
    foo Obj[10];
    mml_list<foo> List = mml_List<foo>(Obj, 10);
    
    cout << "Adding objects to list" << endl;
    for (u32 i = 0; i < 10; ++i)
        mml_Push(&List, { 10 - i });

    for(auto&& Foo : List) {
        cout << Foo.I << ", ";
    }
    cout << endl;
    cout << "Pop!" << endl;
    mml_Pop(&List);
   
    
    for(auto Itr = mml_Begin(&List); Itr != mml_End(&List);) {
        if (Itr->I % 2 == 0) {
            Itr = mml_Remove(&List, Itr);
            continue;
        }
        ++Itr;
    }
    cout << "Removed even numbers" << endl;


        
    for(auto&& Foo : List) {
        cout << Foo.I << ", ";
    }
    cout << endl;
    
    mml_Find(&List, Compare);


    auto FoundItr = mml_Find<foo>(&List, [](const foo* Foo)->b32{
        return Foo->I == 7;
    });
    mml_Remove(&List, FoundItr);

    for(auto&& Foo : List) {
        cout << Foo.I << ", ";
    }
    cout << endl;
    


}
