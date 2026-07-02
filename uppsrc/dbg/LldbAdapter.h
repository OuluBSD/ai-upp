#ifndef _dbg_LldbAdapter_h_
#define _dbg_LldbAdapter_h_

class LldbBackendSession : public DbgBackendSession {
public:
	String GetBackendName() const override;
	DbgRunResult Run(const DbgLaunchRequest& request) override;
};

#endif
