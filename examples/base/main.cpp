#include <iostream>

#include <yal/yal.hpp>

/***************************************************************************/

int main() {
	YAL_CREATE(test1, "test1", 1024*10)
	YAL_CREATE(test2, "test2", 1024*10)
	YAL_CREATE(test3, "test3", 1024*10)
	YAL_CREATE(test4, "test4", 1024*10)
//	YAL_SESSION_TO_TERM(test1, true, "term1")
	for ( auto idx = 0ul, idx2 = 0ul; idx < 1024ul*10ul; ++idx, idx2 += 2 ) {
		YAL_LOG_INFO	(test1, "%1% -> %2% -> %1%", idx, idx+1)
		YAL_LOG_DEBUG	(test1, "%1% -> %2% -> %1%", idx, idx+1)
		YAL_LOG_WARNING(test1, "%1% -> %2% -> %1%", idx, idx+1)
		YAL_LOG_ERROR	(test1, "%1% -> %2% -> %1%", idx, idx+1)
		YAL_SESSION_FLUSH(test1)

		YAL_LOG_INFO	(test2, "%016d -> %016d", idx, idx)
		YAL_LOG_DEBUG	(test2, "%016d -> %016d", idx, idx)
		YAL_LOG_WARNING(test2, "%016d -> %016d", idx, idx)
		YAL_LOG_ERROR	(test2, "%016d -> %016d", idx, idx)
		YAL_SESSION_FLUSH(test2)

		YAL_LOG_INFO	(test3, "%016d -> %016d", idx, idx)
		YAL_LOG_DEBUG	(test3, "%016d -> %016d", idx, idx)
		YAL_LOG_WARNING(test3, "%016d -> %016d", idx, idx)
		YAL_LOG_ERROR	(test3, "%016d -> %016d", idx, idx)
		YAL_SESSION_FLUSH(test3)

		YAL_LOG_INFO	(test4, "%016d -> %016d", idx, idx)
		YAL_LOG_DEBUG	(test4, "%016d -> %016d", idx, idx)
		YAL_LOG_WARNING(test4, "%016d -> %016d", idx, idx)
		YAL_LOG_ERROR	(test4, "%016d -> %016d", idx, idx)
		YAL_SESSION_FLUSH(test4)

		YAL_GLOBAL_LOG_INFO		("%016d -> %016d", idx2, idx2)
		YAL_GLOBAL_LOG_DEBUG		("%016d -> %016d", idx2, idx2)
		YAL_GLOBAL_LOG_WARNING	("%016d -> %016d", idx2, idx2)
		YAL_GLOBAL_LOG_ERROR		("%016d -> %016d", idx2, idx2)
	}
	YAL_FLUSH()

	YAL_SESSION_GET(ts1, "test1")
	assert(ts1);
	YAL_SESSION_GET(ts2, "test2")
	assert(ts2);
	YAL_SESSION_GET(ts3, "test3")
	assert(ts3);
	YAL_SESSION_GET(ts4, "test4")
	assert(ts4);
	YAL_SESSION_GET(ts5, "test5")
	assert(!ts5);

	assert(YAL_SESSION_EXISTS("test1"));
	assert(YAL_SESSION_EXISTS("test2"));
	assert(YAL_SESSION_EXISTS("test3"));
	assert(YAL_SESSION_EXISTS("test4"));
	assert(!YAL_SESSION_EXISTS("test5"));
}

/***************************************************************************/
