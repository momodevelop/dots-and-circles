#include <cmath>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <immintrin.h>

inline float squareRoot(const float val)
{
	return _mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(val)));
}

inline float squareRootSTL(const float val)
{
	return sqrt(val);
}


int main()
{
	float accum = 0.0f;
	unsigned int counter = 1000000000;
	float testVal, testVal2;
	auto begin = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < counter; ++i) {
		testVal2 = 49.48491f + i;
		accum += squareRootSTL(testVal2);
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	std::cout << "accum " <<accum<< std::endl; 
    
	accum = 0.0f;
	auto start2 = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < counter; ++i) {
		testVal = 49.48491f + i;
		accum += squareRoot(testVal);
	}
	auto end2 = std::chrono::high_resolution_clock::now();
	std::cout << testVal << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - begin).count() << " STL version" << std::endl;
	std::cout << testVal2 << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count() << " intrinsics version" << std::endl;
	std::cout << "accum " <<accum<< std::endl; 
	return 0;
}