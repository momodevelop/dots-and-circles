#include <iostream>
#include "ryoji/maths/matrix.h"
#include "ryoji/maths/vector.h"

using namespace std;
using namespace ryoji::maths;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void PrintGoodOrBad(bool good) {
	if (good)
		cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
	else
		cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";
}

template<typename T>
bool SimpleEqualityTest(const char* title, T lhs, T rhs) {
	PrintGoodOrBad(lhs == rhs);
	cout << title << endl;
	return lhs == rhs;
}

void TestMatrix() {
	{
		cout << "=== Testing arithmetic assignment operators" << endl;
		SimpleEqualityTest("Addition", Mat33i{ 1,2,3,4,5,6,7,8,9 } +Mat33i{ 9,8,7,6,5,4,3,2,1 }, Mat33i{ 10,10,10,10,10,10,10,10,10 });
		SimpleEqualityTest("Subtraction", Mat33i{ 1,2,3,4,5,6,7,8,9 } -Mat33i{ 1,2,3,4,5,6,7,8,9 }, Mat33i{ 0,0,0,0,0,0,0,0,0 });
		SimpleEqualityTest("Multiplication", Mat33i{ 1,0,1,0,1,1,0,0,1 } *Mat33i{ 2,0,0,0,2,0,0,0,1 }, Mat33i{ 2,0,1,0,2,1,0,0,1 });
		cout << endl;
	}

	{
		using namespace matrix;
		cout << "=== Testing functions" << endl;
		SimpleEqualityTest("Identity", identity<int, 3, 3>(), Mat33i{ 1,0,0,0,1,0,0,0,1 });
	}
}
void TestVector() {
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
		using namespace vector;
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

}

int main() {
#if 1
	TestVector();
#endif

#if 1
	TestMatrix();
#endif

	return 0;
}