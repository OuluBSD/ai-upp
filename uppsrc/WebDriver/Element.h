#ifndef _WebDriver_Element_h_
#define _WebDriver_Element_h_

#include <Core/Core.h>
#include "detail.h"



NAMESPACE_UPP

// An element from DOM
class Element : public Moveable<Element> { // copyable
public:
	Element();

	Element(
		const String& ref,
		const detail::Shared<detail::Resource>& resource,
		const detail::Shared<detail::IFinder_factory>& factory
		);

	String Get_ref() const; // Returns ID that is used by Webdriver to identify elements

	bool Is_displayed() const;
	bool Is_enabled() const;
	bool Is_selected() const;
	Point Get_location() const;
	Point Get_location_in_view() const;
	Size Get_size() const;
	String Get_attribute(const String& name) const;
	String Get_css_property(const String& name) const;
	String Get_tag_name() const;
	String Get_text() const;

	Element Find_element(const By& by) const;
	Vector<Element> Find_elements(const By& by) const;

	const Element& Clear() const;
	const Element& Click() const;
	const Element& Submit() const;

	const Element& Send_keys(const String& keys) const;
	const Element& Send_keys(const Shortcut& shortcut) const;

	bool Equals(const Element& other) const;
	bool operator != (const Element& other) const;
	bool operator == (const Element& other) const;
	bool operator < (const Element& other) const;

private:
	detail::Resource& Get_resource() const;
	detail::Keyboard Get_keyboard() const;

private:
	String ref_;
	detail::Shared<detail::Resource> resource_;
	detail::Shared<detail::IFinder_factory> factory_;
};

END_UPP_NAMESPACE

#endif