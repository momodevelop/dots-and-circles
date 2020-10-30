#include "../code/mm_core.h"

#include <iostream>
#include <vector>


struct test {
    u32 I;
};

int main() {
    using namespace std;

    vector<test> List;
    
    for( u32 i = 0; i < 10; ++i)
        List.push_back(test{i});
   
    test* Test1 = &List[5];
    cout << "Obtained: " << Test1 << ": " << Test1->I << endl;

    cout << "Before: " << endl;
    for (u32 i = 0; i < List.size(); ++i) {
        cout << &List[i] << ": " << i << ": " <<  List[i].I << endl;;
    }
    
    List.erase(List.begin());

    cout << "After: " << endl;
    for (u32 i = 0; i < List.size(); ++i) {
        cout << &List[i] << ": " << i << ": " << List[i].I << endl;;
    }
    cout << endl;

    test* Test2 = &List[5];

    cout << Test1 << ": " << Test1->I << endl;
    
    cout << Test2 << ": " << Test2->I << endl;

    
}
