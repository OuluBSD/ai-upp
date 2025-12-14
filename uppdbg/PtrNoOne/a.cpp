// This causes: error: cannot initialize a parameter of type 'PteBase *' with an lvalue of type 'A *'
#include <Core/Core.h>
using namespace Upp;

class A {};

Ptr<A> Create() {
	return new A;
}

inline void fn() {
	Ptr<A> owned = Create();
}
