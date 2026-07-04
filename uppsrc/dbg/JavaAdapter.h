#ifndef _dbg_JavaAdapter_h_
#define _dbg_JavaAdapter_h_

class JavaBackendSession : public DbgBackendSession {
public:
	String GetBackendName() const override;
	DbgRunResult Run(const DbgLaunchRequest& request) override;
};

#endif
