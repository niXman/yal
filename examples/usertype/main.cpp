#include <yal/yal.hpp>

#include <iostream>

/***************************************************************************/

struct user_point {
	int x, y;

	friend std::ostream& operator<< (std::ostream &os, const user_point &p) {
		return os << "{x:" << p.x << ",y:" << p.y << '}';
	}
};

/***************************************************************************/

int main() {
	YAL_CREATE(session1, "sessions/session1")
	YAL_SESSION_TO_TERM(session1, true, "terminal")

	user_point p = {34, 65};
	YAL_LOG_INFO(session1, "p=%1%", p)
}

/***************************************************************************/
