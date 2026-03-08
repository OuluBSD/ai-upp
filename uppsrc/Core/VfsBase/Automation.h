#ifndef _Core_VfsBase_Automation_h_
#define _Core_VfsBase_Automation_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct AutomationElement : Moveable<AutomationElement> {
	String  path;
	String  text;
	::Upp::Value   value;
	bool    checked = false;
	bool    enabled = true;
	bool    visible = true;
	bool    is_menu = false;
	
	String ToString() const { return String().Cat() << path << " = " << value << (visible ? "" : " (hidden)"); }
};

class AutomationVisitor : virtual public Visitor {
protected:
	AutomationElement& AddElement(const char *text);

public:
	typedef AutomationVisitor CLASSNAME;

	virtual Visitor& AccessAction(const char *text, Event<> cb) override;
	virtual Visitor& AccessOption(bool check, const char *text, Event<> cb) override;
	virtual Visitor& AccessMenu(const char *text, Event<Visitor&> proc) override;
	virtual Visitor& AccessLabel(const char *text) override;
	virtual Visitor& AccessValue(const ::Upp::Value& v) override;

	Array<AutomationElement> elements;
	String                   current_path;
	
	// Config
	bool                     include_hidden = false;
	
	// Write mode
	bool                     write_mode = false;
	String                   target_path;
	::Upp::Value              target_value;
	bool                     target_action = true;
	bool                     found = false;

	AutomationVisitor();
	virtual ~AutomationVisitor() {}
};

END_UPP_NAMESPACE

#endif