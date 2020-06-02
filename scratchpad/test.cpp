#include <iostream>
using namespace std;


#ifdef PRECISE
#define PRECISION_TYPE        double
#else
#define PRECISION_TYPE        float
#endif
PRECISION_TYPE hollyConstant = 0.017453292519943295769236907684886;
//First of all sine and cosine tables

PRECISION_TYPE sinTable[] = {
    0.0,                                    //sin(0)
    0.17364817766693034885171662676931 ,    //sin(10)
    0.34202014332566873304409961468226 ,    //sin(20)
    0.5 ,                                    //sin(30)
    0.64278760968653932632264340990726 ,    //sin(40)
    0.76604444311897803520239265055542 ,    //sin(50)
    0.86602540378443864676372317075294 ,    //sin(60)
    0.93969262078590838405410927732473 ,    //sin(70)
    0.98480775301220805936674302458952 ,    //sin(80)
    1.0                                     //sin(90)
};

PRECISION_TYPE cosTable[] = {
    1.0 ,                                    //cos(0)
    0.99984769515639123915701155881391 ,    //cos(1)
    0.99939082701909573000624344004393 ,    //cos(2)
    0.99862953475457387378449205843944 ,    //cos(3)
    0.99756405025982424761316268064426 ,    //cos(4)
    0.99619469809174553229501040247389 ,    //cos(5)
    0.99452189536827333692269194498057 ,    //cos(6)
    0.99254615164132203498006158933058 ,    //cos(7)
    0.99026806874157031508377486734485 ,    //cos(8)
    0.98768834059513772619004024769344         //cos(9)
};
// sin (a+b) = sin(a)*cos(b) + sin(b)*cos(a)
// a = 10*m where m is a natural number and 0<= m <= 90
// i.e. lets a+b = 18.22
// then a = 10, b = 8.22

PRECISION_TYPE myFastSin ( PRECISION_TYPE angle )
{
    int a = angle * 0.1f;
    PRECISION_TYPE b = angle - 10 * a;
    
    return sinTable[a] * cosTable[int(b)] + b * hollyConstant * sinTable[9-a];
}


int main() {
    printf("%f", myFastSin(45));
}


