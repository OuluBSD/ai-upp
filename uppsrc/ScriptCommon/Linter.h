#ifndef _ScriptCommon_Linter_h_
#define _ScriptCommon_Linter_h_

class Linter {
public:
	struct Message : Moveable<Message> {
		int line = 0;
		int column = 0;
		String text;
		bool is_error = false;
	};

	Vector<Message> Analyze(const String& code, const String& filename);
};

#endif
