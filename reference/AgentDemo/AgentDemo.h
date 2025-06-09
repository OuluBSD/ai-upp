#ifndef _AgentDemo_AgentDemo_h_
#define _AgentDemo_AgentDemo_h_

#include <AICore2/AICore.h>

NAMESPACE_UPP


struct SimpleValueNode : VfsValueExt {
	
	DEFAULT_EXT(SimpleValueNode)
	void Visit(Vis& v) override {}
	String ToString() const override {return AsString(val.value);}
	double GetUtility() override;
	bool TerminalTest() override;
};


// Class which only only tells the utility value
struct SimpleGeneratorNode : VfsValueExt {
	int value = 0;
	
	DEFAULT_EXT(SimpleGeneratorNode)
	void Visit(Vis& v) override {}
	String ToString() const override {return IntStr(value);}
	double GetUtility() override {return value;}
	void GenerateSubValues(NodeRoute& prev) override;
	bool TerminalTest() override;
};


// Class which tells length of route from the root to the node
struct RouteGeneratorNode : VfsValueExt {
	double length;
	double length_to_node;
	double estimate_to_goal;
	static const int goal = 0;
	
	DEFAULT_EXT(RouteGeneratorNode)
	void Visit(Vis& v) override {}
	String ToString() const override {return DblStr(length) + ", " + DblStr(length_to_node) + ", " + DblStr(estimate_to_goal);}
	double GetUtility() override {return length_to_node;}
	double GetEstimate() override {return estimate_to_goal;}
	double GetDistance(VfsValue& n) override {
		RouteGeneratorNode* rgn = CastPtr<RouteGeneratorNode>(&*n.ext);
		ASSERT(rgn);
		return rgn->length; // no links, so this is always the parent
	}
	void GenerateSubValues(NodeRoute& prev) override;
	bool TerminalTest() override;
};



END_UPP_NAMESPACE

#endif
