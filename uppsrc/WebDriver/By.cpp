#include "WebDriver.h"

NAMESPACE_UPP

By::By(const String& method, const String& selector)
	: method_(method)
	, selector_(selector)
{
}

By By::Id(const String& id) {
	return By("id", id);
}

By By::Name(const String& name) {
	return By("name", name);
}

By By::Class_name(const String& class_name) {
	return By("class name", class_name);
}

By By::Tag_name(const String& tag_name) {
	return By("tag name", tag_name);
}

By By::Css_selector(const String& css_selector) {
	return By("css selector", css_selector);
}

By By::Xpath(const String& xpath) {
	return By("xpath", xpath);
}

By By::Link_text(const String& link_text) {
	return By("link text", link_text);
}

By By::Partial_link_text(const String& partial_link_text) {
	return By("partial link text", partial_link_text);
}

By By::Text(const String& text) {
	return By("text", text);
}

const String& By::Get_method() const {
	return method_;
}

const String& By::Get_selector() const {
	return selector_;
}

END_UPP_NAMESPACE