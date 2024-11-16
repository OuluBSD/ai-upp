#ifndef _AI_TextCtrl_TokenData_h_
#define _AI_TextCtrl_TokenData_h_

NAMESPACE_UPP

// TODO rename
class TokensPage : public ToolAppCtrl {
	Splitter hsplit;
	ArrayCtrl tokens;
	
public:
	typedef TokensPage CLASSNAME;
	TokensPage();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void Do(int fn);
	
	
};


END_UPP_NAMESPACE

#endif
