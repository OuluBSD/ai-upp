// This incorrect change removes the compiling error, but causes silent memory error, where "new A" is never deleted.
#include <Core/Core.h>
using namespace Upp;

class A : public Pte<A> {};

Ptr<A> Create() {
	return new A;
}

inline void fn() {
	Ptr<A> owned = Create();
}
