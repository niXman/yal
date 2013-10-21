
#include <iostream>

#include <yal/yal.hpp>

/***************************************************************************/

int main() {
	auto test1 = yal::logger::create("test1", 1024);
	auto test2 = yal::logger::create("test2", 1024);
	auto test3 = yal::logger::create("test3", 1024);
	auto test4 = yal::logger::create("test4", 1024);
//	YAL_TO_TERM(test1, true, "term1");
	for ( auto idx = 0ul; idx < 1024ul*1024ul*100ul; ++idx ) {
		YAL_INFO		(test1, "%1% -> %2% -> %1%", idx, (idx+1));
		YAL_DEBUG	(test1, "%1% -> %2% -> %1%", idx, (idx+1));
		YAL_WARNING	(test1, "%1% -> %2% -> %1%", idx, (idx+1));
		YAL_ERROR	(test1, "%1% -> %2% -> %1%", idx, (idx+1));
		YAL_INFO		(test2, "%016d -> %016d", idx, idx);
		YAL_DEBUG	(test2, "%016d -> %016d", idx, idx);
		YAL_WARNING	(test2, "%016d -> %016d", idx, idx);
		YAL_ERROR	(test2, "%016d -> %016d", idx, idx);
		YAL_INFO		(test3, "%016d -> %016d", idx, idx);
		YAL_DEBUG	(test3, "%016d -> %016d", idx, idx);
		YAL_WARNING	(test3, "%016d -> %016d", idx, idx);
		YAL_ERROR	(test3, "%016d -> %016d", idx, idx);
		YAL_INFO		(test4, "%016d -> %016d", idx, idx);
		YAL_DEBUG	(test4, "%016d -> %016d", idx, idx);
		YAL_WARNING	(test4, "%016d -> %016d", idx, idx);
		YAL_ERROR	(test4, "%016d -> %016d", idx, idx);
	}
}

/***************************************************************************/
