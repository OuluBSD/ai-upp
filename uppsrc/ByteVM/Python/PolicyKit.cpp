#include "PolicyKit.h"

NAMESPACE_UPP

PolicyKit::PolicyKit()
{
#ifdef flagEMSCRIPTEN
	allow_process_exec = false;
	allow_network = false; // Depends on Emscripten's fetch support, but usually restricted
#endif

#ifdef flagUWP
	allow_process_exec = false;
	// UWP has very restricted filesystem access
#endif
}

bool PolicyKit::Check(PyPermission perm) const
{
	switch(perm) {
	case PYPERM_FILE_READ: return allow_file_read;
	case PYPERM_FILE_WRITE: return allow_file_write;
	case PYPERM_PROCESS_EXEC: return allow_process_exec;
	case PYPERM_NETWORK: return allow_network;
	case PYPERM_ENVIRONMENT: return allow_environment;
	}
	return false;
}

void PolicyKit::Set(PyPermission perm, bool allow)
{
	switch(perm) {
	case PYPERM_FILE_READ: allow_file_read = allow; break;
	case PYPERM_FILE_WRITE: allow_file_write = allow; break;
	case PYPERM_PROCESS_EXEC: allow_process_exec = allow; break;
	case PYPERM_NETWORK: allow_network = allow; break;
	case PYPERM_ENVIRONMENT: allow_environment = allow; break;
	}
}

END_UPP_NAMESPACE
