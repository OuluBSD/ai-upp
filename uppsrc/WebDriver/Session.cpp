#include "WebDriver.h"

NAMESPACE_UPP

Session::Session(const detail::Shared<detail::Resource>& resource)
	: resource_(resource)
	, factory_(new detail::Element_factory())
{
}

Capabilities Session::Get_capabilities() const {
	return resource_->Get_value<Capabilities>("");
}

String Session::Get_source() const {
	return resource_->Get_string("source");
}

String Session::Get_title() const {
	return resource_->Get_string("title");
}

String Session::Get_url() const {
	return resource_->Get_string("url");
}

String Session::Get_screenshot() const {
	return resource_->Get_string("screenshot");
}

const Session& Session::Navigate(const String& url) const {
	resource_->Post("url", "url", url);
	return *this;
}

const Session& Session::Get(const String& url) const {
	return Navigate(url);
}

const Session& Session::Forward() const {
	resource_->Post("forward");
	return *this;
}

const Session& Session::Back() const {
	resource_->Post("back");
	return *this;
}

const Session& Session::Refresh() const {
	resource_->Post("refresh");
	return *this;
}

Element Session::Get_active_element() const {
	detail::Finder finder(resource_, factory_);
	return finder.Find_element(By::Css_selector(":focus")); // Simplified, actual active element might need different command
}

Element Session::Find_element(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.Find_element(by);
}

Vector<Element> Session::Find_elements(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.Find_elements(by);
}

void Session::Delete_session() const {
	resource_->Delete();
}

END_UPP_NAMESPACE