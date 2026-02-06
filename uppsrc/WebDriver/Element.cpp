#include "WebDriver.h"

NAMESPACE_UPP

Element::Element()
	: ref_()
{
}

Element::Element(
	const String& ref,
	const detail::Shared<detail::Resource>& resource,
	const detail::Shared<detail::IFinder_factory>& factory
	)
	: ref_(ref)
	, resource_(resource)
	, factory_(factory)
{
}

String Element::Get_ref() const {
	return ref_;
}

bool Element::Is_displayed() const {
	return resource_->Get_bool("displayed");
}

bool Element::Is_enabled() const {
	return resource_->Get_bool("enabled");
}

bool Element::Is_selected() const {
	return resource_->Get_bool("selected");
}

Point Element::Get_location() const {
	return From_json<Point>(resource_->Get("location"));
}

Point Element::Get_location_in_view() const {
	return From_json<Point>(resource_->Get("location_in_view"));
}

Size Element::Get_size() const {
	return From_json<Size>(resource_->Get("size"));
}

String Element::Get_attribute(const String& name) const {
	return resource_->Get_value<String>("attribute/" + name);
}

String Element::Get_css_property(const String& name) const {
	return resource_->Get_value<String>("css/" + name);
}

String Element::Get_tag_name() const {
	return resource_->Get_string("name");
}

String Element::Get_text() const {
	return resource_->Get_string("text");
}

Element Element::Find_element(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.Find_element(by);
}

Vector<Element> Element::Find_elements(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.Find_elements(by);
}

const Element& Element::Clear() const {
	resource_->Post("clear");
	return *this;
}

const Element& Element::Click() const {
	resource_->Post("click");
	return *this;
}

const Element& Element::Submit() const {
	resource_->Post("submit");
	return *this;
}

const Element& Element::Send_keys(const String& keys) const {
	resource_->Post("value", "value", keys);
	return *this;
}

const Element& Element::Send_keys(const Shortcut& shortcut) const {
	// Convert shortcut to string representation and send
	String keys;
	for (const auto& key : shortcut.keys) {
		keys += key;
	}
	Send_keys(keys);
	return *this;
}

bool Element::Equals(const Element& other) const {
	return ref_ == other.ref_;
}

bool Element::operator!=(const Element& other) const {
	return !(*this == other);
}

bool Element::operator==(const Element& other) const {
	return Equals(other);
}

bool Element::operator<(const Element& other) const {
	return ref_ < other.ref_;
}

detail::Resource& Element::Get_resource() const {
	return *resource_;
}

detail::Keyboard Element::Get_keyboard() const {
	return detail::Keyboard();
}

END_UPP_NAMESPACE