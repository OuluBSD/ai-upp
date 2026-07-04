#ifndef _ide_DebuggersCore_DebuggersCore_h_
#define _ide_DebuggersCore_DebuggersCore_h_

#include <Core/Core.h>

namespace Upp {

struct DbgFrame : Moveable<DbgFrame> {
	String address;
	String function;
	String file;
	int line = Null;
};

struct DbgBacktrace : Moveable<DbgBacktrace> {
	Vector<DbgFrame> frames;
	String error;
};

DbgBacktrace GetGdbBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript);
DbgBacktrace GetLldbBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript);
DbgBacktrace GetJavaBacktrace(const String& exe, const Vector<String>& args, const String& cwd, const VectorMap<String, String>& env, String& transcript);

}

#endif
