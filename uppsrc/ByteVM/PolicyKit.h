#ifndef _ByteVM_PolicyKit_h_
#define _ByteVM_PolicyKit_h_

#include <Core/Core.h>

NAMESPACE_UPP

enum PyPermission {
	PYPERM_FILE_READ,
	PYPERM_FILE_WRITE,
	PYPERM_PROCESS_EXEC,
	PYPERM_NETWORK,
	PYPERM_ENVIRONMENT,
};

class PolicyKit {
	bool allow_file_read = true;
	bool allow_file_write = true;
	bool allow_process_exec = true;
	bool allow_network = true;
	bool allow_environment = true;

public:
	PolicyKit();
	
	bool Check(PyPermission perm) const;
	void Set(PyPermission perm, bool allow);
	
	static PolicyKit& Get() { static PolicyKit pk; return pk; }
};

END_UPP_NAMESPACE

#endif
