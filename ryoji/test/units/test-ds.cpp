#include <iostream>
#include "ryoji/ds/sstring.h"
#include <string.h>
using namespace std;
using namespace ryoji;
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

namespace {
	void PrintGoodOrBad(bool good) {
		if (good)
			cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
		else
			cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";

	}

	namespace test_sstring {
		using namespace ds;
		void Test() {
			std::cout << "=== Testing SString" << std::endl;
			SString<56> str1, str2;
			SString<64> longstr;
			SString<8> shortstr;

			{	
				std::cout << "= Testing functions" << std::endl;
				
				// set
				sstring::set(str1, "Hello World");
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World"));
				std::cout << "set for cstring is correct" << std::endl;

				sstring::set(str2, str1);
				PrintGoodOrBad(!strcmp(sstring::get(str2), "Hello World"));
				std::cout << "set for sstring (same capacity) is correct" << std::endl;

				sstring::set(longstr, str1);
				PrintGoodOrBad(!strcmp(sstring::get(longstr), "Hello World"));
				std::cout << "set for sstring (bigger capacity) is correct" << std::endl;

				sstring::set(shortstr, str1);
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "set for sstring (smaller capacity) is correct" << std::endl;

				sstring::set(str1, sstring::set(shortstr, sstring::set(longstr, "Goodbye World")));
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Goodbye"));
				std::cout << "set chaining is correct" << std::endl;

				// clear
				sstring::clear(str1);
				PrintGoodOrBad(!strcmp(sstring::get(str1), ""));
				std::cout << "clear is correct" << std::endl;

				//concat
				sstring::concat(str1, "Hello ");
				sstring::concat(str1, "World");
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World"));
				std::cout << "concat for cstring is correct" << std::endl;

				sstring::set(str2, " Yo");
				sstring::concat(str1, str2);
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World Yo"));
				std::cout << "concat for sstring is correct" << std::endl;

				sstring::set(str1, "Hello");
				sstring::concat(str1, str1);
				PrintGoodOrBad(!strcmp(sstring::get(str1), "HelloHello"));
				std::cout << "concat for sstring with itself is correct" << std::endl;

				sstring::set(shortstr, "Hello");
				sstring::concat(shortstr, " World");
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "concat short circuit for cstring is correct" << std::endl;

				sstring::set(shortstr, "Hello");
				sstring::set(str1, " World");
				sstring::concat(shortstr, str1);
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "concat short circuit for sstring is correct" << std::endl;

				sstring::clear(str1);
				sstring::concat(str1, sstring::concat(str1, "Hello"));
				PrintGoodOrBad(!strcmp(sstring::get(str1), "HelloHello"));
				std::cout << "concat chain is correct" << std::endl;

				// isEmpty 
				sstring::set(str1, "Test");
				PrintGoodOrBad(!sstring::isEmpty(str1));
				std::cout << "isEmpty is correctly false" << std::endl;
				sstring::clear(str1);
				PrintGoodOrBad(sstring::isEmpty(str1));
				std::cout << "isEmpty is correctly true" << std::endl;

				//compare cstring
				sstring::set(str1, "Hello World");
				PrintGoodOrBad(sstring::isEqual(str1, "Hello World"));
				std::cout << "isEqual on cstring is correctly true" << std::endl;
				
				PrintGoodOrBad(!sstring::isEqual(str1, "Hello Win"));
				std::cout << "isEqual on cstring is correctly false " << std::endl;

				// compare sstring
				sstring::set(str1, "Hello World");
				sstring::set(str2, "Hello World");
				PrintGoodOrBad(sstring::isEqual(str1, str2));
				std::cout << "isEqual on sstring is correctly true" << std::endl;

				sstring::set(str2, "Hello W");
				PrintGoodOrBad(!sstring::isEqual(str1, str2));
				std::cout << "isEqual on sstring is correctly false (based on different length)" << std::endl;

				sstring::set(str2, "Hello Worlx");
				PrintGoodOrBad(!sstring::isEqual(str1, str2));
				std::cout << "isEqual on sstring is correctly false (based on same length but diff content)" << std::endl;

				// join
				sstring::set(str1, "Hello ");
				str2 = sstring::join(str1, "World");
				PrintGoodOrBad(sstring::isEqual(str2, "Hello World"));
				std::cout << "join on cstring is correct" << std::endl;

				sstring::set(str1, "Hello ");
				sstring::set(str2, "World");
				SString str3 = sstring::join(str1, str2);
				PrintGoodOrBad(sstring::isEqual(str3, "Hello World"));
				std::cout << "join on sstring (same capacity) is correct" << std::endl;


				sstring::set(str1, "Hello ");
				SString<5> str4 = sstring::join<5>(str1, " World");
				PrintGoodOrBad(sstring::isEqual(str4, "Hell"));
				std::cout << "join on cstring (different capacity output) is correct" << std::endl;

				sstring::set(str1, "Hello ");
				sstring::set(longstr, "World");
				SString str5 = sstring::join<5>(str1, longstr);
				PrintGoodOrBad(sstring::isEqual(str5, "Hell"));
				std::cout << "join on sstring (different capacity input/output) is correct" << std::endl;



			}
			std::cout << std::endl;
			{
				std::cout << "= Testing operator overloading" << std::endl;
				
				// operator=
				str1 = "Hello World";
				sstring::set(str1, "Hello World");
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World"));
				std::cout << "operator= for cstring is correct" << std::endl;

				str2 = str1;
				PrintGoodOrBad(!strcmp(sstring::get(str2), "Hello World"));
				std::cout << "operator= for sstring (same capacity) is correct" << std::endl;

				longstr = str1;
				PrintGoodOrBad(!strcmp(sstring::get(longstr), "Hello World"));
				std::cout << "operator= for sstring (bigger capacity) is correct" << std::endl;

				shortstr = str1;
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "operator= for sstring (smaller capacity) is correct" << std::endl;

				str1 = shortstr = longstr = "Goodbye World";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Goodbye"));
				std::cout << "operator= chaining is correct" << std::endl;

				// operator+=
				str1 = "Hello ";
				str1 += "World";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World"));
				std::cout << "operator+= for cstring is correct" << std::endl;

				str2 = " Yo";
				str1 += str2;
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World Yo"));
				std::cout << "operator+= for sstring is correct" << std::endl;

				str1 = "Hello";
				str1 += str1;
				PrintGoodOrBad(!strcmp(sstring::get(str1), "HelloHello"));
				std::cout << "operator+= for sstring with itself is correct" << std::endl;

				shortstr = "Hello";
				shortstr += " World";
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "operator+= short circuit for cstring is correct" << std::endl;

				shortstr = "Hello";
				str1 = " World";
				shortstr += str1;
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "operator+= short circuit for sstring is correct" << std::endl;

				str1 = "Do";
				str2 = "Re";
				longstr = "Mi";
				shortstr = "Fa";
				str1 += str2 += longstr += shortstr += "SoLa";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "DoReMiFaSoLa"));
				std::cout << "operator+= chain is correct (1)" << std::endl;


				str1 = "Do";
				shortstr = "Re";
				str2 = "Mi";
				longstr = "Fa";
				str1 += shortstr += str2 += longstr += "SoLa";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "DoReMiFaS"));
				std::cout << "operator+= chain is correct (2)" << std::endl;

				// operator<<
				str1 = "Hello ";
				str1 << "World";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "Hello World"));
				std::cout << "operator<< for cstring is correct" << std::endl;

				str2 = "Gerald: ";
				str2 << str1;
				PrintGoodOrBad(!strcmp(sstring::get(str2), "Gerald: Hello World"));
				std::cout << "operator<< for sstring is correct" << std::endl;

				str1 = "Hello";
				str1 << str1;
				PrintGoodOrBad(!strcmp(sstring::get(str1), "HelloHello"));
				std::cout << "operator<< for sstring with itself is correct" << std::endl;

				shortstr = "Hello";
				shortstr << " World";
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "operator+= short circuit for cstring is correct" << std::endl;

				shortstr = "Hello";
				str1 = " World";
				shortstr << str1;
				PrintGoodOrBad(!strcmp(sstring::get(shortstr), "Hello W"));
				std::cout << "operator+= short circuit for sstring is correct" << std::endl;

				str1 = "Do";
				str2 = "Re";
				longstr = "Mi";
				shortstr = "Fa";
				str1 << str2 << longstr << shortstr << "SoLa";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "DoReMiFaSoLa"));
				std::cout << "operator<< chain is correct (1)" << std::endl;

				str1 = "Do";
				shortstr = "Re";
				str2 = "Mi";
				longstr = "Fa";
				str1 << shortstr << str2 << longstr << "SoLa";
				PrintGoodOrBad(!strcmp(sstring::get(str1), "DoReMiFaSoLa"));
				std::cout << "operator<< chain is correct (2)" << std::endl;

				// operator == and !=
				//compare cstring
				str1 = "Hello World";
				PrintGoodOrBad(str1 == "Hello World");
				std::cout << "operator== on cstring is correct" << std::endl;

				PrintGoodOrBad(str1 !="Hello Win");
				std::cout << "operator!= on cstring is correct " << std::endl;

				// compare sstring
				str1 = longstr = "Hello World";
				PrintGoodOrBad(str1 == longstr);
				std::cout << "operator== on sstring is correct" << std::endl;

				str2 = "Hello W";
				PrintGoodOrBad(str1 != str2);
				std::cout << "operator!= on sstring is correct" << std::endl;

				PrintGoodOrBad(str1[0] == 'H');
				std::cout << "operator[] is correct" << std::endl;

				const auto& constStr1  = str1;
				PrintGoodOrBad(constStr1[0] == 'H');
				std::cout << "operator[] (const) is correct" << std::endl;

				PrintGoodOrBad(!strcmp((*str1), "Hello World"));
				std::cout << "operator* is correct" << std::endl;

				PrintGoodOrBad(!strcmp((*constStr1), "Hello World"));
				std::cout << "operator* (const) is correct" << std::endl;

				// operator+
				str1 = "Hello";
				PrintGoodOrBad(!strcmp(*(str1 + " World"), "Hello World"));
				std::cout << "operator+ for cstring is correct" << std::endl;

				str2 = " World";
				PrintGoodOrBad(!strcmp(*(str1 + str2), "Hello World"));
				std::cout << "operator+ for sstring is correct" << std::endl;
			}
		}
	}


}

int TestDS() {
	test_sstring::Test();
	return 0;
}

#undef CONCAT