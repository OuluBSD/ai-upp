#ifndef _dbg_BackendFactory_h_
#define _dbg_BackendFactory_h_

class DbgBackendSession;

One<DbgBackendSession> CreateDbgBackendSession(const String& backend_name);

#endif
