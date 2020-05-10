#include <iostream>
#include "ryoji/maths/matrices.h"
#include "ryoji/maths/vectors.h"
#include "ryoji/maths/bitwise.h"

using namespace ryoji::maths;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void PrintGoodOrBad(bool good, const char * str) {
	using namespace std;
	if (good)
		cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
	else
		cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";
	cout << str << endl;
}

template<typename T>
bool SimpleEqualityTest(const char* title, T lhs, T rhs) {
	PrintGoodOrBad(lhs == rhs, title);
	return lhs == rhs;
}

void TestMtxrix() {
	using namespace std;
	using namespace matrices;
	cout << "===== Testing Mtxrix =====" << endl;
	{
		
		cout << "=== Testing arithmetic assignment operators" << endl;
		SimpleEqualityTest("Addition", Mtx33i{ 1,2,3,4,5,6,7,8,9 } +Mtx33i{ 9,8,7,6,5,4,3,2,1 }, Mtx33i{ 10,10,10,10,10,10,10,10,10 });
		SimpleEqualityTest("Subtraction", Mtx33i{ 1,2,3,4,5,6,7,8,9 } -Mtx33i{ 1,2,3,4,5,6,7,8,9 }, Mtx33i{ 0,0,0,0,0,0,0,0,0 });
		SimpleEqualityTest("Multiplication", Mtx33i{ 1,0,1,0,1,1,0,0,1 } *Mtx33i{ 2,0,0,0,2,0,0,0,1 }, Mtx33i{ 2,0,1,0,2,1,0,0,1 });
		cout << endl;
	}

	{
		cout << "=== Testing functions" << endl;
		SimpleEqualityTest("Identity", identity<int, 3, 3>(), Mtx33i{ 1,0,0,0,1,0,0,0,1 });
		cout << endl;
	}

	cout << endl;
}
void TestVector() {
	using namespace vectors;
	using namespace std;
	cout << "===== Testing Vector =====" << endl;
	/*{
		cout << "=== Testing create" << endl;
		auto vec4i = create<int, 4>(1,2,3,4 );
		SimpleEqualityTest("Creation", vec4i, Vec4i{ 1,2,3,4 });
		cout << endl;
	}*/

	{

		cout << "=== Testing arithmetic operators" << endl;
		Vec4i vec4{ 1, 2, 3, 4 };
		SimpleEqualityTest("Addition", vec4 + Vec4i{ 1,1,1,1 }, Vec4i{ 2,3,4,5 });
		SimpleEqualityTest("Subtraction", vec4 - Vec4i{ 1,1,1,1 }, Vec4i{ 0,1,2,3 });
		SimpleEqualityTest("Multiplication", vec4 * Vec4i{ 2,2,2,2 }, Vec4i{ 2,4,6,8 });
		SimpleEqualityTest("Division", vec4 / Vec4i{ 4,4,4,4 }, Vec4i{ 0,0,0,1 });
		SimpleEqualityTest("Modulo", vec4 % Vec4i{ 4,4,4,4 }, Vec4i{ 1,2,3,0 });
		SimpleEqualityTest("Scalar Multiplication", vec4 * 2, Vec4i{ 2,4,6,8 });
		SimpleEqualityTest("Scalar Division", vec4 / 4, Vec4i{ 0,0,0,1 });
		SimpleEqualityTest("Scalar Modulo", vec4 % 4, Vec4i{ 1,2,3,0 });
		SimpleEqualityTest("Reverse Scalar Multiplication", 2 * vec4, Vec4i{ 2,4,6,8 });
		SimpleEqualityTest("Negation", -vec4, Vec4i{ -1,-2,-3,-4 });
		cout << endl;
	}

	{
		cout << "=== Testing arithmetic assignment operators" << endl;
		Vec4i vec4{ 1, 2, 3, 4 };
		SimpleEqualityTest("Addition", vec4 += Vec4i{ 1,1,1,1 }, Vec4i{ 2,3,4,5 });
		SimpleEqualityTest("Subtraction", vec4 -= Vec4i{ 1,1,1,1 }, Vec4i{ 1,2,3,4 });
		SimpleEqualityTest("Multiplication", vec4 *= Vec4i{ 2,2,2,2 }, Vec4i{ 2,4,6,8 });
		SimpleEqualityTest("Division", vec4 /= Vec4i{ 2,2,2,2 }, Vec4i{ 1,2,3,4 });
		SimpleEqualityTest("Scalar Multiplication", vec4 *= 2, Vec4i{ 2,4,6,8 });
		SimpleEqualityTest("Scalar Division", vec4 /= 2, Vec4i{ 1,2,3,4 });
		cout << endl;
	}

	{
		using namespace vectors;
		cout << "=== Testing functions" << endl;
		SimpleEqualityTest("Dot Product", dot(Vec4f{ 1,2,3,4 }, Vec4f{ 1,2,3,4 }), 30.0f);
		SimpleEqualityTest("Midpoint", midpoint(Vec4f{ 3,3,3,3 }, Vec4f{ 1,1,1,1 }), Vec4f{ 2,2,2,2 });
		SimpleEqualityTest("DistanceSq", distanceSq(Vec4f{ 2,2,2,2 }, Vec4f{ 4,4,4,4 }), 16.0f);
		SimpleEqualityTest("Distance", distance(Vec4f{ 2,2,2,2 }, Vec4f{ 4,4,4,4 }), 4.0f);
		SimpleEqualityTest("LengthSq", lengthSq(Vec4f{ 2,2,2,2 }), 16.0f);
		SimpleEqualityTest("Length", length(Vec4f{ 2,2,2,2 }), 4.0f);
		SimpleEqualityTest("Normalize", length(normalize(Vec4f{ 1,1,1,1 })), 1.0f);
		SimpleEqualityTest("IsPerpendicular", isPerpendicular(Vec4f{ -1,1,0,0 }, Vec4f{ 1,1,0,0 }), true);
		SimpleEqualityTest("IsOppositeDirection", isOppositeDirection(Vec4f{ -1.5f,-1.5f,0,0 }, Vec4f{ 1,1,0,0 }), true);
		SimpleEqualityTest("IsSimilarDirection", isSimilarDirection(Vec4f{ 1.5f,1.5f,0,0 }, Vec4f{ 1,1,0,0 }), true);
		SimpleEqualityTest("AngleBetween", utils::radiansToDegrees(angleBetween(Vec4f{ 1,0,0,0 }, Vec4f{ 1,1,0,0 })), 45.0f);
		SimpleEqualityTest("NormalizedVectorsAngleBetween", utils::radiansToDegrees(normalized::angleBetween(Vec4f{ 1,0,0,0 }, Vec4f{ 0,1,0,0 })), 90.0f);
		SimpleEqualityTest("Convert (to int)", convert<int>(Vec4f{ 1,2,3,4 }), Vec4i{ 1,2,3,4 });
		SimpleEqualityTest("Convert (to float)", convert<float>(Vec4i{ 1,2,3,4 }), Vec4f{ 1,2,3,4 });
		SimpleEqualityTest("Projection", normalize(project(Vec4f{ 1,2,3,4 }, Vec4f{ 1,1,1,1 })), normalize(Vec4f{ 1,1,1,1 }));

	}
	cout << endl;

}

void TestBitwise() {
	using namespace bitfields;
	std::cout << "===== Testing Bitwise =====" << std::endl;
	SimpleEqualityTest("bitfield", bitfield<int>(0, 1, 2, 3, 4, 5, 6, 7), 0b11111111);
	SimpleEqualityTest("mask", mask(0b001, 0b010), 3);
	SimpleEqualityTest("unmask", unmask(0b101, 0b001), 4);
	SimpleEqualityTest("any (true)", any(0b101, 0b011), true);
	SimpleEqualityTest("any (false)", any(0b101, 0b010), false);
	SimpleEqualityTest("all (true)", all(0b101, 0b101), true);
	SimpleEqualityTest("all (false)", all(0b101, 0b111), false);
	SimpleEqualityTest("get (true)", get(0b101, 2), true);
	SimpleEqualityTest("get (false)", get(0b101, 1), false);
	SimpleEqualityTest("set (turn on)", set(0b101, 1, true), 0b111);
	SimpleEqualityTest("set (turn off)", set(0b111, 1, false), 0b101);
}

int main() {
#if 1
	TestVector();
#endif

#if 1
	TestMtxrix();
#endif

#if 1
	TestBitwise();
#endif

	return 0;
}