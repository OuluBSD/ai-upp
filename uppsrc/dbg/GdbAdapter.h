#ifndef _dbg_GdbAdapter_h_
#define _dbg_GdbAdapter_h_

class GdbBackendSession : public DbgBackendSession {
public:
	String GetBackendName() const override;
	DbgRunResult Run(const DbgLaunchRequest& request) override;
};

#endif
