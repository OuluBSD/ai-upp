#ifndef _ide_ConsoleHost_h_
#define _ide_ConsoleHost_h_

class IdeCoreConsoleHost {
public:
	IdeCoreConsoleHost();

	int  Execute(const char *cmdline, String *output = NULL, const char *envptr = NULL, bool quiet = true);
	bool Run(const char *cmdline, const char *envptr = NULL);
	bool IsRunning() const;
	int  Wait(String *output = NULL);
	void Kill();

private:
	One<AProcess> process;
	String        buffer;
	int           exit_code = -1;
};

#endif
