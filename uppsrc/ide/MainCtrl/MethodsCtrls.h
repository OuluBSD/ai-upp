#ifndef _ide_MethodsCtrls_h_
#define _ide_MethodsCtrls_h_

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#else
#include <Core/Core.h>
#endif

namespace Upp {

#ifdef flagGUI

class TextOption : public Option {
public:
	virtual void   SetData(const Value& data);
	virtual Value  GetData() const;
};

class TextSwitch : public Switch {
public:
	virtual void   SetData(const Value& data);
	virtual Value  GetData() const;
};

#else

class TextOption {
public:
	void   SetData(const Value& data) { value = data; }
	Value  GetData() const            { return value; }

private:
	Value value;
};

class TextSwitch {
public:
	void   SetData(const Value& data) { value = data; }
	Value  GetData() const            { return value; }

private:
	Value value;
};

#endif

}

#endif
