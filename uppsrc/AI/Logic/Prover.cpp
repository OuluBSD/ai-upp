#include "TheoremProver.h"
#include <AI/Algo/FastSearch.h>

namespace TheoremProver {

int GetIndexCommonCount(const ArrayMap<NodeVar, int>& a, const ArrayMap<NodeVar, int>& b) {
	int count = 0;
	for(int i = 0; i < a.GetCount(); i++) {
		if (b.Find(a.GetKey(i)) != -1)
			count++;
	}
	return count;
}

void RemoveRef(ArrayMap<NodeVar, int>& ind, const NodeVar& ref) {
	int i = ind.Find(ref);
	if (i != -1)
		ind.Remove(i);
}

int& GetInsert(ArrayMap<NodeVar, int>& data, const NodeVar& ref) {
	int i = data.Find(ref);
	if (i != -1)
		return data[i];
	return data.Insert(0, ref);
}

// Unification

// solve a single equation
ArrayMap<NodeVar, NodeVar> Unify(Node& term_a, Node& term_b) {
	UnificationTerm* uniterm_a = dynamic_cast<UnificationTerm*>(&term_a);
	UnificationTerm* uniterm_b = dynamic_cast<UnificationTerm*>(&term_b);
	ArrayMap<NodeVar, NodeVar> out;
	
	if (uniterm_a) {
		if (term_b.Occurs(*uniterm_a) || term_b.GetTime() > term_a.GetTime())
			return out;
		
		out.Add(&term_a, &term_b);
		return out;
	}

	if (uniterm_b) {
		if (term_a.Occurs(*uniterm_b) || term_a.GetTime() > term_b.GetTime())
			return out;

		out.Add(&term_b, &term_a);
		return out;
	}

	if (dynamic_cast<Variable*>(&term_a) && dynamic_cast<Variable*>(&term_b)) {
		if (term_a == term_b)
			return { };

		return out;
	}

	Function* fn_a = dynamic_cast<Function*>(&term_a);
	Function* fn_b = dynamic_cast<Function*>(&term_b);
	Predicate* pred_a = dynamic_cast<Predicate*>(&term_a);
	Predicate* pred_b = dynamic_cast<Predicate*>(&term_b);
	
	if ((fn_a && fn_b) || (pred_a && pred_b)) {
		if (term_a.GetName() != term_b.GetName())
			return out;
		
		bool is_fn = (fn_a && fn_b);
		Index<NodeVar>& a_terms = is_fn ? fn_a->terms : pred_a->terms;
		Index<NodeVar>& b_terms = is_fn ? fn_b->terms : pred_b->terms;
		
		if (a_terms.GetCount() != b_terms.GetCount())
			return out;

		ArrayMap<NodeVar, NodeVar> substitution;

		for (int i = 0; i < a_terms.GetCount(); i++) {
			NodeVar a = a_terms[i];
			NodeVar b = b_terms[i];
			
			for (int j = 0; j < substitution.GetCount(); j++) {
				const NodeVar& k = substitution.GetKey(j);
				NodeVar& v = substitution[j];
				a = a->Replace(*k, *v);
				b = b->Replace(*k, *v);
			}

			ArrayMap<NodeVar, NodeVar> sub = Unify(*a, *b);

			if (sub.GetCount() == 0)
				return out;
			
			for(int j = 0; j < sub.GetCount(); j++) {
				const NodeVar& k = sub.GetKey(j);
				NodeVar& v = sub[j];
				substitution.Add(k, v);
			}
		}

		return substitution;
	}

	return out;
}

// solve a list of equations
ArrayMap<NodeVar, NodeVar> UnifyList(const ArrayMap<NodeVar, NodeVar>& pairs) {
	ArrayMap<NodeVar, NodeVar> substitution;
	
	for (int i = 0; i < pairs.GetCount(); i++) {
		const NodeVar& term_a = pairs.GetKey(i);
		const NodeVar& term_b = pairs[i];
		NodeVar a = term_a;
		NodeVar b = term_b;
		
		for(int j = 0; j < substitution.GetCount(); j++) {
			const NodeVar& k = substitution.GetKey(j);
			NodeVar& v = substitution[j];
			a = a->Replace(*k, *v);
			b = b->Replace(*k, *v);
		}

		ArrayMap<NodeVar, NodeVar> sub = Unify(*a, *b);

		if (sub.GetCount() == 0)
			return ArrayMap<NodeVar, NodeVar>();
		
		for(int j = 0; j < sub.GetCount(); j++) {
			const NodeVar& k = sub.GetKey(j);
			NodeVar& v = sub[j];
			substitution.Add(k, v);
		}
	}

	return substitution;
}

// Sequents
class Sequent : public Node {
	
public:
	friend bool ProveSequent(Node& sequent);
	
	ArrayMap<NodeVar, int> left, right;
	Index<NodeVar> siblings;
	int depth;
	
public:
	Sequent(const ArrayMap<NodeVar, int>& left, const ArrayMap<NodeVar, int>& right, const Index<NodeVar>& siblings, int depth) : Node("") {
		this->left <<= left;
		this->right <<= right;
		this->siblings <<= siblings;
		this->depth = depth;
	}

	virtual Index<NodeVar> FreeVariables() {
		Index<NodeVar> result;

		for(int i = 0; i < left.GetCount(); i++)
			Append(result, left.GetKey(i)->FreeVariables());
		
		for(int i = 0; i < right.GetCount(); i++)
			Append(result, right.GetKey(i)->FreeVariables());
		
		return result;
	}

	virtual Index<NodeVar> FreeUnificationTerms() {
		Index<NodeVar> result;

		for(int i = 0; i < left.GetCount(); i++)
			Append(result, left.GetKey(i)->FreeUnificationTerms());
		
		for(int i = 0; i < right.GetCount(); i++)
			Append(result, right.GetKey(i)->FreeUnificationTerms());
		
		return result;
	}

	String GetVariableName(const String& prefix) {
		Index<NodeVar> fv;
		Append(fv, FreeVariables());
		Append(fv, FreeUnificationTerms());
		int index = 1;
		String name = prefix + IntStr(index);

		NodeVar var_ref = new Variable(name);
		NodeVar unterm_ref = new UnificationTerm(name);
		
		while (fv.Find(var_ref) != -1 || fv.Find(unterm_ref) != -1) {
			index += 1;
			name = prefix + IntStr(index);
			var_ref = new Variable(name);
			unterm_ref = new UnificationTerm(name);
		}
		
		return name;
	}

	ArrayMap<NodeVar, NodeVar> GetUnifiablePairs() {
		ArrayMap<NodeVar, NodeVar> pairs;

		for (int i = 0; i < left.GetCount(); i++) {
			const NodeVar& formula_left = left.GetKey(i);
			for (int j = 0; j < right.GetCount(); j++) {
				const NodeVar& formula_right = right.GetKey(j);
				ArrayMap<NodeVar, NodeVar> tmp = Unify(*formula_left, *formula_right);
				if (tmp.GetCount())
					pairs.Add(formula_left, formula_right);
			}
		}

		return pairs;
	}

	virtual bool operator==(Node& other) {
		Sequent* seq = dynamic_cast<Sequent*>(&other);
		
		for (int i = 0; i < left.GetCount(); i++) {
			if (seq->left.Find(left.GetKey(i)) == -1)
				return false;
		}

		for (int i = 0; i < seq->left.GetCount(); i++) {
			if (left.Find(seq->left.GetKey(i)) == -1)
				return false;
		}

		for (int i = 0; i < right.GetCount(); i++) {
			if (seq->right.Find(right.GetKey(i)) == -1)
				return false;
		}

		for (int i = 0; i < seq->right.GetCount(); i++) {
			if (right.Find(seq->right.GetKey(i)) == -1)
				return false;
		}

		return true;
	}

	virtual String ToString() const {
		String left_part, right_part;
		
		for(int i = 0; i < left.GetCount(); i++) {
			if (i) left_part += ", ";
			left_part += left.GetKey(i)->ToString();
		}
		
		for(int i = 0; i < right.GetCount(); i++) {
			if (i) right_part += ", ";
			right_part += right.GetKey(i)->ToString();
		}

		if (left_part != "")
			left_part = left_part + " ";

		if (right_part != "")
			right_part = " " + right_part;

		return left_part + "⊢" + right_part;
	}
	
	bool IsAxiom() {
		// Intersection of left and right
		for(int i = 0; i < left.GetCount(); i++) {
			for(int j = 0; j < right.GetCount(); j++) {
				if (left.GetKey(i) == right.GetKey(j)) {
					RLOG("  IsAxiom: Found match!");
					RLOG("    Left[" << i << "]: " << left.GetKey(i)->ToString());
					RLOG("    Right[" << j << "]: " << right.GetKey(j)->ToString());
					return true;
				}
			}
		}
		
		// Reflexivity: t = t on the right
		for(int i = 0; i < right.GetCount(); i++) {
			Equal* eq = dynamic_cast<Equal*>(&*right.GetKey(i));
			if (eq && *eq->left == *eq->right)
				return true;
		}
		
		return false;
	}
	
	int GetComplexity() const {
		int c = 0;
		for(int i = 0; i < left.GetCount(); i++) c += left.GetKey(i)->ToString().GetCount();
		for(int i = 0; i < right.GetCount(); i++) c += right.GetKey(i)->ToString().GetCount();
		return c;
	}
	
Index<NodeVar> lemma_cache;

	void Expand(Vector<NodeVar>& out) {
		RLOG("  Expand sequent: " << ToString());
		// Equality Reasoning: Substitution rule (Left)
		// if we have t1 = t2 on the left, we can replace occurrences of t1 with t2 in other formulas
		for(int i = 0; i < left.GetCount(); i++) {
			Equal* eq = dynamic_cast<Equal*>(&*left.GetKey(i));
			if (eq) {
				for(int j = 0; j < left.GetCount(); j++) {
					if (i == j) continue;
					const NodeVar& target = left.GetKey(j);
					if (target->OccursIn(*eq->left)) {
						Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
						new__sequent->Inc();
						NodeVar updated = target->Replace(*eq->left, *eq->right);
						if (new__sequent->left.Find(updated) == -1) {
							RemoveRef(new__sequent->left, target);
							GetInsert(new__sequent->left, updated) = left[j] + 1;
							out.Add(new__sequent);
						}
						new__sequent->Dec();
					}
				}
			}
		}

		// determine which formula to expand
		NodeVar left_formula;
		int left_depth = -1;

		for (int i = 0; i < left.GetCount(); i++) {
			const NodeVar& formula = left.GetKey(i);
			int d = left[i];
			
			if (left_depth == -1 || left_depth > d) {
				if (!dynamic_cast<Predicate*>(&*formula) && !dynamic_cast<Equal*>(&*formula)) {
					left_formula = formula;
					left_depth = d;
				}
			}
		}

		NodeVar right_formula;
		int right_depth = -1;

		for (int i = 0; i < right.GetCount(); i++) {
			const NodeVar& formula = right.GetKey(i);
			int d = right[i];
			
			if (right_depth == -1 || right_depth > d) {
				if (!dynamic_cast<Predicate*>(&*formula) && !dynamic_cast<Equal*>(&*formula)) {
					right_formula = formula;
					right_depth = d;
				}
			}
		}
	
		bool apply_left = false;
		bool apply_right = false;

		if (left_formula.Is() && !right_formula.Is())
			apply_left = true;

		if (!left_formula.Is() && right_formula.Is())
			apply_right = true;

		if (left_formula.Is() && right_formula.Is()) {
			if (left_depth < right_depth)
				apply_left = true;
			else
				apply_right = true;
		}

		if (!left_formula.Is() && !right_formula.Is())
			return;

		// apply a left rule
		if (apply_left) {
			Not* not_ = dynamic_cast<Not*>(&*left_formula);
			if (not_) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->left, left_formula);
				GetInsert(new__sequent->right, not_->formula) = (left.Get(left_formula) + 1);
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			And* and_ = dynamic_cast<And*>(&*left_formula);
			if (and_) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->left, left_formula);
				GetInsert(new__sequent->left, and_->formula_a) = (left.Get(left_formula) + 1);
				GetInsert(new__sequent->left, and_->formula_b) = (left.Get(left_formula) + 1);
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			Or* or_ = dynamic_cast<Or*>(&*left_formula);
			if (or_) {
				Sequent* new__sequent_a = new Sequent(left, right, siblings, depth + 1);
				Sequent* new__sequent_b = new Sequent(left, right, siblings, depth + 1);
				new__sequent_a->Inc();
				new__sequent_b->Inc();
				RemoveRef(new__sequent_a->left, left_formula);
				RemoveRef(new__sequent_b->left, left_formula);
				GetInsert(new__sequent_a->left, or_->formula_a) = (left.Get(left_formula) + 1);
				GetInsert(new__sequent_b->left, or_->formula_b) = (left.Get(left_formula) + 1);

				if (new__sequent_a->siblings.GetCount()) new__sequent_a->siblings.Insert(0, new__sequent_a);
				if (new__sequent_b->siblings.GetCount()) new__sequent_b->siblings.Insert(0, new__sequent_b);

				out.Add(new__sequent_a);
				out.Add(new__sequent_b);
				new__sequent_a->Dec();
				new__sequent_b->Dec();
				return;
			}
			
			Implies* implies = dynamic_cast<Implies*>(&*left_formula);
			if (implies) {
				Sequent* new__sequent_a = new Sequent(left, right, siblings, depth + 1);
				Sequent* new__sequent_b = new Sequent(left, right, siblings, depth + 1);
				new__sequent_a->Inc();
				new__sequent_b->Inc();
				RemoveRef(new__sequent_a->left, left_formula);
				RemoveRef(new__sequent_b->left, left_formula);
				GetInsert(new__sequent_a->right, implies->formula_a) = (left.Get(left_formula) + 1);
				GetInsert(new__sequent_b->left,  implies->formula_b) = (left.Get(left_formula) + 1);

				if (new__sequent_a->siblings.GetCount()) new__sequent_a->siblings.Insert(0, new__sequent_a);
				if (new__sequent_b->siblings.GetCount()) new__sequent_b->siblings.Insert(0, new__sequent_b);

				out.Add(new__sequent_a);
				out.Add(new__sequent_b);
				new__sequent_a->Dec();
				new__sequent_b->Dec();
				return;
			}
			
			ForAll* forall = dynamic_cast<ForAll*>(&*left_formula);
			if (forall) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				new__sequent->left.Get(left_formula) += 1;
				NodeVar unterm = new UnificationTerm(GetVariableName("t"));
				NodeVar formula = forall->formula->Replace(*forall->variable, *unterm);
				formula->SetInstantiationTime(depth + 1);
				SortByKey(new__sequent->left, NodeVar());
				if (new__sequent->left.Find(formula) == -1)
					new__sequent->left.Add(formula, new__sequent->left.Get(left_formula));
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			ThereExists* there_exists = dynamic_cast<ThereExists*>(&*left_formula);
			if (there_exists) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->left, left_formula);
				NodeVar variable = new Variable(GetVariableName("v"));
				NodeVar formula = there_exists->formula->Replace(*there_exists->variable, *variable);
				formula->SetInstantiationTime(depth + 1);
				GetInsert(new__sequent->left, formula) = (left.Get(left_formula) + 1);
				SortByKey(new__sequent->left, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
		}

		// apply a right rule
		if (apply_right) {
			Not* not_ = dynamic_cast<Not*>(&*right_formula);
			if (not_) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->right, right_formula);
				GetInsert(new__sequent->left, not_->formula) = (right.Get(right_formula) + 1);
				SortByKey(new__sequent->left, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			And* and_ = dynamic_cast<And*>(&*right_formula);
			if (and_) {
				Sequent* new__sequent_a = new Sequent(left, right, siblings, depth + 1);
				Sequent* new__sequent_b = new Sequent(left, right, siblings, depth + 1);
				RemoveRef(new__sequent_a->right, right_formula);
				RemoveRef(new__sequent_b->right, right_formula);
				GetInsert(new__sequent_a->right, and_->formula_a) = right.Get(right_formula) + 1;
				GetInsert(new__sequent_b->right, and_->formula_b) = right.Get(right_formula) + 1;
				SortByKey(new__sequent_a->right, NodeVar());
				SortByKey(new__sequent_b->right, NodeVar());
				if (new__sequent_a->siblings.GetCount()) new__sequent_a->siblings.Insert(0, new__sequent_a);
				if (new__sequent_b->siblings.GetCount()) new__sequent_b->siblings.Insert(0, new__sequent_b);
				out.Add(new__sequent_a);
				out.Add(new__sequent_b);
				return;
			}
			
			Or* or_ = dynamic_cast<Or*>(&*right_formula);
			if (or_) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->right, right_formula);
				GetInsert(new__sequent->right, or_->formula_a) = right.Get(right_formula) + 1;
				GetInsert(new__sequent->right, or_->formula_b) = right.Get(right_formula) + 1;
				SortByKey(new__sequent->right, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			Implies* implies = dynamic_cast<Implies*>(&*right_formula);
			if (implies) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->right, right_formula);
				GetInsert(new__sequent->left,  implies->formula_a) = right.Get(right_formula) + 1;
				GetInsert(new__sequent->right, implies->formula_b) = right.Get(right_formula) + 1;
				SortByKey(new__sequent->right, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			ForAll* forall = dynamic_cast<ForAll*>(&*right_formula);
			if (forall) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				RemoveRef(new__sequent->right, right_formula);
				NodeVar variable(new Variable(GetVariableName("v")));
				NodeVar formula = forall->formula->Replace(*forall->variable, *variable);
				formula->SetInstantiationTime(depth + 1);
				GetInsert(new__sequent->right, formula) = right.Get(right_formula) + 1;
				SortByKey(new__sequent->right, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
			
			ThereExists* there_exists = dynamic_cast<ThereExists*>(&*right_formula);
			if (there_exists) {
				Sequent* new__sequent = new Sequent(left, right, siblings, depth + 1);
				new__sequent->Inc();
				new__sequent->right.Get(right_formula) += 1;
				NodeVar uniterm = new UnificationTerm(GetVariableName("t"));
				NodeVar formula = there_exists->formula->Replace(*there_exists->variable, *uniterm);
				formula->SetInstantiationTime(depth + 1);
				if (new__sequent->right.Find(formula) == -1)
					new__sequent->right.Insert(0, formula, new__sequent->right.Get(right_formula));
				SortByKey(new__sequent->right, NodeVar());
				new__sequent->siblings.Insert(0, new__sequent);
				out.Add(new__sequent);
				new__sequent->Dec();
				return;
			}
		}
	}

};

class SequentGenerator : public FastSearchGenerator<NodeVar> {
public:
	void Generate(const NodeVar& node, Vector<NodeVar>& out) override {
		Sequent* seq = dynamic_cast<Sequent*>(&*node);
		if (seq) seq->Expand(out);
	}
	bool IsGoal(const NodeVar& node) override {
		Sequent* seq = dynamic_cast<Sequent*>(&*node);
		return seq && seq->IsAxiom();
	}
	double GetEstimate(const NodeVar& node) override {
		Sequent* seq = dynamic_cast<Sequent*>(&*node);
		if (!seq) return 1000.0;
		return (double)seq->GetComplexity();
	}
};


// Proof search

struct AStarSequent : Moveable<AStarSequent> {
	NodeVar sequent;
	double g = 0;
	double f = 0;
	
	bool operator<(const AStarSequent& other) const { return f > other.f; }
};

// returns true if the sequent == provable
// returns false || loops forever if the sequent != provable
Index<NodeVar> conflict_cache;

bool ProveSequent(Node& sequent_, int max_depth = 20) {
	Sequent& sequent = dynamic_cast<Sequent&>(sequent_);
	
	// Use AStar for simple proofs without siblings
	if (sequent.siblings.IsEmpty()) {
		Vector<AStarSequent> open_set;
		AStarSequent s;
		s.sequent = &sequent;
		s.g = 0;
		s.f = (double)sequent.GetComplexity();
		open_set.Add(s);
		
		Index<NodeVar> visited;
		int count = 0;
		while(open_set.GetCount() > 0 && count++ < 5000) {
			int best_idx = 0;
			for(int i = 1; i < open_set.GetCount(); i++)
				if (open_set[i].f < open_set[best_idx].f)
					best_idx = i;
			
			AStarSequent current = open_set[best_idx];
			open_set.Remove(best_idx);
			
			if (conflict_cache.Find(current.sequent) != -1) continue;
			
			Sequent* cur_seq = current.sequent.Get<Sequent>();
			if (cur_seq->IsAxiom())
				return true;
			
			if (current.g >= max_depth) continue;
			
			if (visited.Find(current.sequent) != -1) continue;
			visited.Add(current.sequent);
			
			Vector<NodeVar> next;
			cur_seq->Expand(next);
			
			if (next.IsEmpty()) {
				conflict_cache.Add(current.sequent);
				continue;
			}
			
			for(int i = 0; i < next.GetCount(); i++) {
				const NodeVar& n = next[i];
				if (visited.Find(n) != -1 || conflict_cache.Find(n) != -1) continue;
				
				AStarSequent next_node;
				next_node.sequent = n;
				next_node.g = current.g + 1.0;
				next_node.f = next_node.g + (double)n.Get<Sequent>()->GetComplexity();
				open_set.Add(next_node);
			}
		}
	}

	// Fallback to manual Best-First Search for complex unification
	// reset the time for each formula in the sequent
	for (int i = 0; i < sequent.left.GetCount(); i++)
		sequent.left.GetKey(i)->SetInstantiationTime(0);

	for (int i = 0; i < sequent.right.GetCount(); i++)
		sequent.right.GetKey(i)->SetInstantiationTime(0);

	// sequents to be proven
	Vector<NodeVar> frontier;
	frontier.Add(&sequent);
	
	//# sequents which have been proven
	Index<NodeVar> proven;
	
	String prev_str;
	
	while (true) {
		// Sort frontier by complexity (Best-First Search)
		if (frontier.GetCount() > 1) {
			Sort(frontier, [](const NodeVar& a, const NodeVar& b) {
				Sequent* sa = dynamic_cast<Sequent*>(&*a);
				Sequent* sb = dynamic_cast<Sequent*>(&*b);
				return sa->GetComplexity() < sb->GetComplexity();
			});
		}
		
		// get the next sequent
		NodeVar old_sequent_;

		while (frontier.GetCount() > 0) {
			old_sequent_ = frontier[0];
			frontier.Remove(0);
			if (proven.Find(old_sequent_) == -1)
				break;
			old_sequent_ = NodeVar();
		}

		if (old_sequent_.Is() == false)
			break;
		
		if (conflict_cache.Find(old_sequent_) != -1) continue;
		
		Sequent* old_sequent = old_sequent_.Get<Sequent>();
		ASSERT(old_sequent);
		String seq_str = old_sequent->ToString();
		if (seq_str == prev_str || old_sequent->depth > max_depth) {
			conflict_cache.Add(old_sequent_);
			Log(LOG_SEARCH, LL_WARN, "Unable to continue: depth limit or cycle detected");
			return false;
		}
		prev_str = seq_str;
		Log(LOG_SEARCH, LL_INFO, Format("%d. %s", old_sequent->depth, seq_str));

		// check if this sequent == axiomatically true
		if (old_sequent->IsAxiom()) {
			proven.Add(old_sequent);
			continue;
		}

		// check if this sequent has unification terms
		if (old_sequent->siblings.GetCount()) {
			
			// get the unifiable pairs for each sibling
			Vector<ArrayMap<NodeVar, NodeVar> > sibling_pair_lists;
			for(int i = 0; i < old_sequent->siblings.GetCount(); i++) {
				sibling_pair_lists.Add(old_sequent->siblings[i].Get<Sequent>()->GetUnifiablePairs());
			}

			// check if there == a unifiable pair for each sibling
			bool all_has_count = true;
			for(int i = 0; i < sibling_pair_lists.GetCount(); i++)
				if (sibling_pair_lists[i].GetCount() == 0)
					all_has_count = false;
			
			if (all_has_count) {
				
				// iterate through all simultaneous choices of pairs from each sibling
				ArrayMap<NodeVar, NodeVar> substitution;
				Vector<int> index;
				index.SetCount(sibling_pair_lists.GetCount(), 0);

				while (true) {
					// attempt to unify at the index
					ArrayMap<NodeVar, NodeVar> tmp;
					for(int i = 0; i < sibling_pair_lists.GetCount(); i++) {
						int j = index[i];
						tmp.Add(sibling_pair_lists[i].GetKey(j), sibling_pair_lists[i][j]);
					}
					substitution = UnifyList(tmp);

					if (substitution.GetCount())
						break;

					// increment the index
					int pos = sibling_pair_lists.GetCount() - 1;

					while (pos >= 0) {
						index[pos] += 1;

						if (index[pos] < sibling_pair_lists[pos].GetCount())
							break;

						index[pos] = 0;
						pos -= 1;
					}

					if (pos < 0)
						break;
				}

				if (substitution.GetCount()) {
					for(int i = 0; i < substitution.GetCount(); i++) {
						const NodeVar& k = substitution.GetKey(i);
						const NodeVar& v = substitution[i];
						Log(LOG_UNIFICATION, LL_DEBUG, Format("  %s = %s", k->ToString(), v->ToString()));
					}
					
					Append(proven, old_sequent->siblings);
					
					for(int i = 0; i < frontier.GetCount(); i++) {
						if (old_sequent->siblings.Find(frontier[i]) != -1) {
							frontier.Remove(i);
							i--;
						}
					}
					//frontier = [sequent for sequent in frontier if sequent !in old_sequent->siblings];
					continue;
				}
			}
			else {
				// unlink this sequent
				while (1) {
					int i = old_sequent->siblings.Find(old_sequent);
					if (i != -1)
						old_sequent->siblings.Remove(i);
					else
						break;
				}
				//old_sequent->siblings.remove(old_sequent);
			}
		}

		Vector<NodeVar> next;
		old_sequent->Expand(next);
		
		if (next.IsEmpty())
			return false;
		
		for(int i = 0; i < next.GetCount(); i++)
			frontier.Add(next[i]);
	}

	// if the frontier is empty, check if the goal sequent is in proven
	return proven.Find(&sequent) != -1;
}

// returns true if the formula == provable
// returns false || loops forever if the formula != provable
bool ProveFormula(const Index<NodeVar>& axioms, const NodeVar& formula) {
	RLOG("ProveFormula started:");
	RLOG("  Formula: " << (formula.Is() ? formula->ToString() : "NULL"));
	RLOG("  Axioms count: " << axioms.GetCount());
	for(int i = 0; i < axioms.GetCount(); i++)
		RLOG("    Axiom[" << i << "]: " << axioms[i]->ToString());

	if (lemma_cache.Find(formula) != -1) {
		RLOG("  FOUND IN LEMMA CACHE!");
		return true;
	}
	conflict_cache.Clear();
	
	ArrayMap<NodeVar, int> left, right;
	for(int i = 0; i < axioms.GetCount(); i++)
		left.Add(axioms[i], 0);
	right.Add(formula, 0);
	
	// Iterative Deepening loop
	for(int max_depth = 5; max_depth <= 30; max_depth += 5) {
		NodeVar seq(new Sequent(left, right, Index<NodeVar>(), 0));
		if (ProveSequent(*seq, max_depth)) {
			lemma_cache.Add(formula);
			return true;
		}
	}
	return false;
}

extern String* catch_print;
extern Index<NodeVar> axioms;
extern ArrayMap<NodeVar, Index<NodeVar> > lemmas;

String ProveLogicNode(NodeVar formula) {
	String out;
	if (!formula.Is()) return out;
	
	ClearProofSteps();
	catch_print = &out;
	
	CheckFormula ( *formula );
	Index<NodeVar> tmp;
	for(int i = 0; i < axioms.GetCount(); i++)
		tmp.Add(axioms[i]);
	for(int i = 0; i < lemmas.GetCount(); i++) {
		int j = tmp.Find(lemmas.GetKey(i));
		if (j == -1) tmp.Add(lemmas.GetKey(i));
	}
	
	bool result = ProveFormula ( tmp, formula );
	ASSERT(formula.GetNode());
	
	if ( result )
		Print ( Format( "Formula proven: %s.", formula->ToString() ));
	else
		Print ( Format( "Formula unprovable: %s.", formula->ToString() ));
	
	catch_print = 0;
	
	return out;
}

String ProveLogic(String str) {
	NodeVar n = Parse(str);
	if (n.Is()) return ProveLogicNode(n);
	return "";
}

}