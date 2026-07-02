#ifndef _dbg_Toolchain_h_
#define _dbg_Toolchain_h_

struct DbgToolchainStatus : Moveable<DbgToolchainStatus> {
	String         backend_name;
	bool           available = false;
	Vector<String> messages;
};

DbgToolchainStatus CheckDbgBackendToolchain(const String& backend_name);

#endif
