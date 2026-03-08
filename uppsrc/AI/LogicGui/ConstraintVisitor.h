#ifndef _AI_LogicGui_ConstraintVisitor_h_
#define _AI_LogicGui_ConstraintVisitor_h_

#include <CtrlCore/CtrlCore.h>
#include <AI/Logic/TheoremProver.h>

namespace Upp {

class ConstraintVisitor : public Visitor {
	Index<String> facts;
	String current_ctrl;
	Ctrl  *ctrl;

public:
	ConstraintVisitor();
	
	virtual Visitor& AccessLabel(const char *text) override;
	virtual Visitor& AccessAction(const char *text, Event<> cb) override;
	virtual Visitor& AccessOption(bool check, const char *text, Event<> cb) override;
	virtual Visitor& AccessValue(const Value& v) override;
	
	void CollectFacts(Ctrl& top);
	const Index<String>& GetFacts() const { return facts; }
};

}

#endif
