#ifndef _dbg_VsAdapter_h_
#define _dbg_VsAdapter_h_

class VsBackendSession : public DbgBackendSession {
public:
	String GetBackendName() const override;
	DbgRunResult Run(const DbgLaunchRequest& request) override;
};

#endif
