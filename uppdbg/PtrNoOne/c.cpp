// This correct change removes the compiling error and has clean RAII exit.
#include <Core/Core.h>
using namespace Upp;

class A {};

One<A> Create() {
	return new A;
}

inline void fn() {
	One<A> owned = Create();
}
