#ifndef __RYOJI_ALLOCATORS_DETAIL_PREDEF_FREELIST_STRATEGIES_H__
#define __RYOJI_ALLOCATORS_DETAIL_PREDEF_FREELIST_STRATEGIES_H__

#include <utility>

namespace ryoji::allocators {
	namespace detail {

		class FirstFitStrategy {
		public:
			// Returns the previous node (as first) and the node that fits (as second) 
			template<class Iterator>
			std::pair<Iterator, Iterator> find(Iterator begin, Iterator end, size_t size) {
				Iterator prev(nullptr);
				while (begin != end) {
					if (size <= (*begin)->size) {
						break;
					}
					prev = begin;
					++begin;
				}

				return { prev, begin };

			}
		};

		class BestFitStrategy {
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