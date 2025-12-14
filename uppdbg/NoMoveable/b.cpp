#include <Core/Core.h>
using namespace Upp;


struct A : Moveable<A> {
	String nontrivial;
};

inline void fn() {
	Vector<A> avec;
	avec.Add();
}
