#ifndef _Maestro_ConversionOrchestrator_h_
#define _Maestro_ConversionOrchestrator_h_

class ConversionOrchestrator {
public:
	static void Inventory(const String& source, const String& target);
	static void Plan(const String& source, const String& target);
	static void Run(const String& source, const String& target, int limit = 0);
	static void Validate(const String& source, const String& target);
};

struct ConvertCommand : Command {
	String GetName() const override { return "convert"; }
	String GetDescription() const override { return "AI-driven conversion orchestration"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI convert [-h] {inventory,plan,validate,run}\n"
		       << "\n"
		       << "Conversion subcommands:\n"
		       << "    inventory <src> <tgt>  Generate source and target inventories\n"
		       << "    plan <src> <tgt>       Generate conversion plan JSON\n"
		       << "    validate <src> <tgt>   Validate conversion plan\n"
		       << "    run <src> <tgt>        Execute conversion plan\n";
	}
	
	void Execute(const Vector<String>& args) override;
};

#endif