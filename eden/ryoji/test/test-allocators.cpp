#include <iostream>
#include <vector>
#include <list>

#include "ryoji/allocators/fallback-allocator.h"
#include "ryoji/allocators/freelist-allocator.h"
#include "ryoji/allocators/heap-allocator.h"
#include "ryoji/allocators/linear-allocator.h"
#include "ryoji/allocators/local-allocator.h"
#include "ryoji/allocators/middleware-allocator.h"
#include "ryoji/allocators/null-allocator.h"
#include "ryoji/allocators/segregator-allocator.h"
#include "ryoji/allocators/slab-allocator.h"
#include "ryoji/allocators/stack-allocator.h"
#include "ryoji/allocators/stl-adapter.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


using namespace std;
using namespace ryoji::allocators;

void PrintGoodOrBad(bool good) {
	if (good)
		cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
	else
		cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";
}

void TestSTLOnVector() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnVector()" << endl;
	vector<int, Allocator> vec1;
	for (int i = 0; i < 10; ++i) {
		vec1.push_back(i);
	}
	bool green = true;
	int i = 0;
	for (auto&& element : vec1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	PrintGoodOrBad(green);

	cout << "Working on std::vector"  << endl;
	cout << endl;
}
void TestSTLOnList() {
	using Allocator = STLAdapter<int, HeapAllocator>;
	cout << "==== TestSTLOnList()" << endl;
	list<int, Allocator> list1;
	for (int i = 0; i < 10; ++i) {
		list1.push_back(i);
	}

	bool green = true;
	int i = 0;
	for (auto&& element : list1) {
		if (element != i++) {
			green = false;
			break;
		}
	}
	PrintGoodOrBad(green);
	cout << "Working on std::list" << endl;
	cout << endl;
}

static bool isFallingback = false;  //eww
template<bool IsFallback>
struct StackLogStrategy {
private:
	char* currentPtr = nullptr;

	void prefixPrint() {
		if constexpr (IsFallback) {
			cout << "[" << ANSI_COLOR_YELLOW << "Fallback" << ANSI_COLOR_RESET << "] ";
		}
		else {
			cout << "[" << ANSI_COLOR_CYAN << "Main" << ANSI_COLOR_RESET << "] ";
		}
	}
public:
	void preAllocate(size_t, uint8_t) {}
	void postAllocate(Blk blk) {
		if (IsFallback != isFallingback) {
			PrintGoodOrBad(!blk);
			prefixPrint();
			cout << "Allocator should not allocate" << endl;
			return;
		}
		
		PrintGoodOrBad(blk);
		prefixPrint();
		cout << "Allocator should allocate" << endl;

		if (currentPtr) {
			cout << "> ";
			PrintGoodOrBad(blk.ptr == currentPtr + 4);
			prefixPrint();
			cout << "The block of memory is 4 bytes away from the previous one @ " << blk.ptr << endl;
		}
		else {
			cout << "> ";
			PrintGoodOrBad(blk);
			prefixPrint();
			cout << "The first block of memory is not null @ " << blk.ptr << endl;
		}
		currentPtr = reinterpret_cast<char*>(blk.ptr);
		
		
		

	}
	void preDeallocate(Blk blk) {
		PrintGoodOrBad(!isFallingback);
		cout << "Stack called to deallocate 4 bytes" << endl;
	}
	void postDeallocate() {}
	void preDeallocateAll() {}
	void postDeallocateAll() {}
	
};

void TestFallbackLocalHeap() {
	cout << "=== Testing Fallback/Local/Heap/Stack allocators" << endl;

	
	isFallingback = false;


	using Allocator = FallbackAllocator<
		MiddlewareAllocator<LocalStackAllocator<50>, StackLogStrategy<false>>,
		MiddlewareAllocator<LocalStackAllocator<50>, StackLogStrategy<true>>>;
		

	Allocator allocator;
	// Stack allocator of 50 bytes should only be able to take 50 / (4 + 1) = 10 4-byte allocations
	// +1 is due to the bookkeeping required per allocator

	for (int i = 0; i < 20; ++i) {
		if (i == 10) {
			cout << endl << "> Fallback should happen here!" << endl << endl;
			isFallingback = true;
		}
		allocator.allocate(4, 4);

	}

	cout << endl;
}

void TestStackAllocator() {
	cout << "=== Testing Stack allocator" << endl;
	// Stack allocator of 100 bytes should only be able to take 100 / (4 + 1) = 20 4-byte allocations
	// +1 is due to the bookkeeping required per allocator
	using Allocator = HeapStackAllocator<100>;
	Allocator allocator;

	auto startBlk = allocator.allocate(4, 4); // store the memory address of first item's allocation
	allocator.deallocate(startBlk);

	vector<Blk> blkList;
	bool allocatesSuccess = true;
	for (int i = 0; i < 20; ++i) {
		auto blk = allocator.allocate(4, 4);
		blkList.push_back(blk);
		if (!blk)
			allocatesSuccess = false;
		
	}
	auto expectNullBlk = allocator.allocate(4, 4);
	PrintGoodOrBad(!expectNullBlk && allocatesSuccess);
	cout << "Allocation integrity is okay" << endl;

	// deallocate test
	for (int i = 19; i >= 0; --i) {
		allocator.deallocate(blkList[i]);
	}
	auto tmpBlk = allocator.allocate(4, 4);

	PrintGoodOrBad(tmpBlk == startBlk);
	cout << "Deallocation integrity is okay" << endl;

	allocator.deallocate(tmpBlk);




	cout << endl;
}

void TestLinearAllocator() {
	cout << "=== Testing Linear allocator" << endl;
	using Allocator = HeapLinearAllocator<100>;
	Allocator allocator;

	bool allocatesSuccess = true;
	for (int i = 0; i < 25; ++i) {
		auto blk = allocator.allocate(4, 4);
		if (!blk)
			allocatesSuccess = false;
	}
	auto expectNullBlk = allocator.allocate(4, 4);

	PrintGoodOrBad(!expectNullBlk && allocatesSuccess);
	cout << "Allocation integrity is okay" << endl;
}
void TestSegregatorAllocator() {
	cout << "=== Testing SegregatorAllocator" << endl;
	using Allocator = SegregatorAllocator<4,
			LocalLinearAllocator<100>,
			LocalLinearAllocator<100>
	>;

	Allocator allocator;

	// the memory given back on the first allocation of the 
	// 'small' LocalLinearAllocator should be at least 100 bytes after the
	// first allocation of the 'big' LocalLinearAllocator.

	auto small = allocator.allocate(4, 4);
	auto large = allocator.allocate(8, 4);

	PrintGoodOrBad(((char*)large.ptr - (char*)small.ptr) >= 100);
	cout << "Allocation integrity is okay" << endl;
	cout << endl;

}


void TestFreeListAllocator() {
	cout << "=== Testing FreeListAllocator" << endl;

	using Allocator = LocalFirstFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk = allocator.allocate(4, 4);
	const char* const firstAllocationPtr = reinterpret_cast<char*>(blk.ptr);
	allocator.deallocate(blk);

	Blk blks[20];
	for (size_t i = 0; i < 20; ++i) {
		blks[i] = allocator.allocate(i + 1, (uint8_t)(pow(2.0, (double)(i % 4))));
		if (!blks[i]) {
			cout << "Failed to allocate..." << endl;
			return;
		}

		(*(char*)((blks[i]).ptr)) = 'A';

		if (i % 4 == 0) {
			allocator.deallocate(blks[i / 2]);
			blks[i / 2] = {};
		}
	}

	for (int i = 0; i < 20; ++i) {
		allocator.deallocate(blks[i]);
	}
	blk = allocator.allocate(4, 4);

	PrintGoodOrBad((char*)blk.ptr == firstAllocationPtr);
	cout << "Allocation integrity is okay" << endl;

	
	cout << endl;
}


void TestFreeListFirstFitAllocator() {
	cout << "=== Testing FreeListFirstFitAllocator" << endl;

	using Allocator = LocalFirstFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk1 = allocator.allocate(4, 4);
	auto blk2 = allocator.allocate(4, 4);
	auto blk3 = allocator.allocate(4, 4);
	auto blk4 = allocator.allocate(4, 4);

	allocator.deallocate(blk2);
	auto blk5 = allocator.allocate(4, 4);

	PrintGoodOrBad((blk5.ptr < blk4.ptr && blk5.ptr > blk1.ptr));
	cout << "First Fit integrity is okay" << endl;
	cout << endl;
}

void TestFreeListBestFitAllocator() {
	cout << "=== Testing FreeListBestFitAllocator" << endl;

	using Allocator = LocalBestFitFreeListAllocator<1000>;
	// We'll do a simple test. Allocate and deallocate semi-randomly.
	// Once everything is deallocated, next allocation should be same as first allocation.
	Allocator allocator;
	auto blk1 = allocator.allocate(4, 4);
	auto blk2 = allocator.allocate(10, 4);
	auto blk3 = allocator.allocate(4, 4);
	auto blk4 = allocator.allocate(4, 4);
	auto blk5 = allocator.allocate(4, 4);

	allocator.deallocate(blk2);
	allocator.deallocate(blk4);
	auto blk6 = allocator.allocate(4, 4);
	PrintGoodOrBad((blk6.ptr > blk3.ptr&& blk6.ptr < blk5.ptr));
	cout << "Best Fit integrity is okay" << endl;
	cout << endl;

}

void TestSlabAllocator() {
	cout << "=== Testing SlabAllocator" << endl;
	using Allocator = SlabAllocator<1000, 25, 4, LocalAllocator<1000>>;
	Allocator allocator;


	/// TODO

}
void TestMiddlewareAllocator() {
	cout << "=== Testing MiddlewareAllocator" << endl;
	
	{
		cout << "= Full Test" << endl;

		struct Middleware {
			bool preAllocated = false;
			bool postAllocated = false;
			bool preDeallocated = false;
			bool postDeallocated = false;

			void preAllocate(size_t, uint8_t) { preAllocated = true; }
			void postAllocate(Blk) { postAllocated = true; }
			void preDeallocate(Blk) { preDeallocated = true; }
			void postDeallocate(Blk) { postDeallocated = true; }
		};


		using Allocator = MiddlewareAllocator<LocalAllocator<1000>, Middleware>;
		Allocator allocator;

		auto middleware = allocator.getMiddleware();
		auto ret = allocator.allocate(10, 4);
		allocator.deallocate(ret);
		
		PrintGoodOrBad(middleware.preAllocated);
		cout << "preAllocated called" << endl;

		PrintGoodOrBad(middleware.postAllocated);
		cout << "preAllocate called" << endl;

		PrintGoodOrBad(middleware.preDeallocated);
		cout << "preDeallocate called" << endl;

		PrintGoodOrBad(middleware.postDeallocated);
		cout << "postDeallocate called" << endl;

	}

	{
		cout << "= Partial Test" << endl;
		struct Middleware {
			bool postAllocated = false;
			bool postDeallocated = false;
			bool postDeallocatedAll = false;

			void postAllocate(Blk) { postAllocated = true; }
			void postDeallocate(Blk) { postDeallocated = true; }
		};

		using Allocator = MiddlewareAllocator<LocalAllocator<1000>, Middleware>;
		Allocator allocator;

		auto middleware = allocator.getMiddleware();
		auto ret = allocator.allocate(10, 4);
		allocator.deallocate(ret);

		PrintGoodOrBad(middleware.postAllocated);
		cout << "preAllocate called" << endl;


		PrintGoodOrBad(middleware.postDeallocated);
		cout << "postDeallocate called" << endl;


	}


	cout << endl;


}

int main() {
	TestMiddlewareAllocator();
	TestSTLOnVector();
	TestSTLOnList();
	TestFallbackLocalHeap();
	TestStackAllocator();
	TestLinearAllocator();
	TestSegregatorAllocator();
	TestFreeListAllocator();
	TestFreeListFirstFitAllocator();
	TestFreeListBestFitAllocator();
	//TestSlabAllocator();
}