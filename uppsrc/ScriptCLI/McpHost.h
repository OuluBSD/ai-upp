#ifndef _ScriptCLI_McpHost_h_
#define _ScriptCLI_McpHost_h_

struct McpHostConfig {
	String workspace;
	String transport;
	int    port = 7326;
};

class McpHost {
public:
	int Run(const McpHostConfig& config);

private:
	int RunStdio(const McpHostConfig& config);
};

#endif
