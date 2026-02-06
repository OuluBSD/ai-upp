#ifndef _WebDriver_By_h_
#define _WebDriver_By_h_

#include <Core/Core.h>

NAMESPACE_UPP

class By { // copyable
public:
	By(const String& method, const String& selector);

	static By Id(const String& id);
	static By Name(const String& name);
	static By Class_name(const String& class_name);
	static By Tag_name(const String& tag_name);
	static By Css_selector(const String& css_selector);
	static By Xpath(const String& xpath);
	static By Link_text(const String& link_text);
	static By Partial_link_text(const String& partial_link_text);
	static By Text(const String& text);

	const String& Get_method() const;
	const String& Get_selector() const;

private:
	String method_;
	String selector_;
};

END_UPP_NAMESPACE

#endif