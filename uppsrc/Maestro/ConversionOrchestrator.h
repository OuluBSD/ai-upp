
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
	Vector<String> GetAliases() const override { return {"c", "cv"}; }
	String GetDescription() const override { return "Format conversion tools and pipelines"; }
	
	void ShowHelp() const override {
		Cout() << "usage: MaestroCLI convert [-h] {add,new,n,plan,p,run,r,status,s,show,sh,reset,rst,batch,b} ...\n"
		       << "positional arguments:\n"
		       << "  {add,new,n,plan,p,run,r,status,s,show,sh,reset,rst,batch,b}\n"
		       << "                        Convert subcommands\n"
		       << "    add (new, n)        Add new conversion pipeline\n"
		       << "    plan (p)            Plan conversion approach\n"
		       << "    run (r)             Run conversion pipeline\n"
		       << "    status (s)          Get pipeline status\n"
		       << "    show (sh)           Show pipeline details\n"
		       << "    reset (rst)         Reset pipeline state\n"
		       << "    batch (b)           Batch operations\n"
		       << "options:\n"
		       << "  -h, --help            show this help message and exit\n";
	}
	
	void Execute(const Vector<String>& args) override;
};

#endif

