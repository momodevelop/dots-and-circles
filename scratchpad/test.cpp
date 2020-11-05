#include "../code/mm_core.h"
#include "../code/mm_string.h"

#include "test.h"
#include <iostream>
#include <vector>
using namespace std;


int main() {

    
    char buffer[11];
    mms_string Text = mms_CreateString(buffer, 12);
    mms_Copy(&Text, "Hello World");
    mms_NullTerm(&Text);

    cout << Text.Length << endl;
    cout << Text.Buffer << endl;

}
