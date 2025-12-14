#include <Core/Core.h>
using namespace Upp;


struct A {
	String nontrivial;
};

CONSOLE_APP_MAIN {
	Vector<A> avec;
	avec.Add();
}
