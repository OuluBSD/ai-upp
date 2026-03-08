
#ifndef _Maestro_DiscussCommand_h_
#define _Maestro_DiscussCommand_h_

struct DiscussCommand : Command {
	String GetName() const override { return "discuss"; }
	String GetDescription() const override { return "Start an AI discussion using the current context"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI discuss [-h] [prompt]\n";
	}
	
	void Execute(const Vector<String>& args) override;
};

#endif

