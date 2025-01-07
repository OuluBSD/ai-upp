#ifndef _DropTerm_CommandPrompt_h_
#define _DropTerm_CommandPrompt_h_

class ConsoleCtrl;

class CommandPrompt : public Upp::CodeEditor {
	ArrayMap<String, EscValue> vars;
	ConsoleCtrl* cons;

public:
	CommandPrompt(ConsoleCtrl* cons);
	void    Execute();

	virtual bool Key(dword key, int count);
	virtual void LeftDouble(Point p, dword flags);

};

ArrayMap<String, EscValue>& UscGlobal();

#endif
