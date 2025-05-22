#include "AICore.h"

NAMESPACE_UPP

double GetSearcherUtility(Nod& n) {
	Value& o = n.value;
	ValueArray arr = o;
	ASSERT(arr.GetCount());
	Value ov = arr[1];
	double value = ov;
	return value;
}

double GetSearcherEstimate(Nod& n) {
	ASSERT(n.ext);
	return n.ext->GetEstimate();
}

double GetSearcherDistance(Nod& n, Nod& dest) {
	ASSERT(n.ext);
	if (n.ext)
		return n.ext->GetDistance(dest);
	else
		return DBL_MAX;
}

bool TerminalTest(Nod& n, Nod** prev) {
	return n.GetCount() == 0;
}









MiniMax::MiniMax() {}
	
double MiniMax::MaxValue(Nod& n, int* decision_pos) {
	Nod* p = 0;
	if (TerminalTest(n,&p))
		return this->Utility(n);
	double v = -DBL_MAX;
	int pos = -1;
	for(int i = 0; i < n.GetTotalCount(); i++) {
		// Find minimum...
		double d = MinValue(n.AtTotal(i));
		// ..but use maximum
		if (d > v) {
			v = d;
			pos = i;
		}
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

double MiniMax::MinValue(Nod& n, int* decision_pos) {
	Node<T>* p = 0;
	if (TerminalTest(n,&p))
		return Searcher<T>::Utility(n);
	double v = DBL_MAX;
	int pos = -1;
	for(int i = 0; i < n.GetTotalCount(); i++) {
		double d = MaxValue(n.AtTotal(i));
		if (d < v) {
			v = d;
			pos = i;
		}
	}
	if (decision_pos) *decision_pos = pos;
	return v;
}

Vector<T*> MiniMax::Search(Nod& src) {
	Vector<T*> out;
	Nod* ptr = &src;
	Nod* prev = 0;
	while (1) {
		T& t = *out.Add((T*)ptr);
		if (TerminalTest(*ptr,&prev)) break;
		int type = out.GetCount() % 2;
		int pos = -1;
		double v;
		if (type == 1)
			v = MinValue(*ptr, &pos);
		else
			v = MaxValue(*ptr, &pos);
		if (pos == -1) break;
		ptr = &(*ptr).AtTotal(pos);
		//LOG(pos << " " << v);
	}
	return out;
}

END_UPP_NAMESPACE
