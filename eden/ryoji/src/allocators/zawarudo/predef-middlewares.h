#ifndef __RYOJI_ALLOCATORS_ZAWARUDO_PREDEF_MIDDLEWARES_H__
#define __RYOJI_ALLOCATORS_ZAWARUDO_PREDEF_MIDDLEWARES_H__

#include <utility>
#include <iostream>
namespace ryoji::allocators {
	namespace zawarudo {

		class LogMiddleware {
		public:
			void preAllocate(size_t, uint8_t) {
				cout << "Al"
			}
			void postAllocate(size_t, uint8_t) {

			}
			void preDeallocate(size_t, uint8_t) {

			}
			void postDeallocate(size_t, uint8_t) {

			}
			void preDeallocateAll(size_t, uint8_t) {

			}
			void postDeallocateAll(size_t, uint8_t) {

			}
				
		};

		class CounterMiddleware {
		public:
			// Returns the previous node (as first) and the node that fits (as second) 
			template<class Iterator>
			std::pair<Iterator, Iterator> find(Iterator begin, Iterator end, size_t size) {
				Iterator prev(nullptr);
				Iterator prevResult(nullptr);
				Iterator result(nullptr);
				size_t bestSize = -1;

				while (begin != end) {
					if (size <= (*begin)->size && (*begin)->size <= bestSize) {
						prevResult = prev;
						result = begin;
						bestSize = (*begin)->size;
					}
					prev = begin;
					++begin;
				}

				return { prevResult, result };

			}
		};
	}
}

#endif