#include "WebDriver.h"

NAMESPACE_UPP

Window::Window() {
}

Window::Window(
	const String& handle,
	const detail::Shared<detail::Resource>& resource
	)
	: handle_(handle)
	, resource_(resource)
{
}

String Window::Get_handle() const {
	return handle_;
}

Size Window::Get_size() const {
	return From_json<Size>(resource_->Get("size"));
}

const Window& Window::Set_size(const Size& size) const {
	resource_->Post("size", To_json(size));
	return *this;
}

Point Window::Get_position() const {
	return From_json<Point>(resource_->Get("position"));
}

const Window& Window::Set_position(const Point& position) const {
	resource_->Post("position", To_json(position));
	return *this;
}

const Window& Window::Maximize() const {
	resource_->Post("maximize");
	return *this;
}

const Window& Window::Fullscreen() const {
	resource_->Post("fullscreen");
	return *this;
}

const Window& Window::Close() const {
	resource_->Delete();
	return *this;
}

String Window::Get_title() const {
	return resource_->Get_string("title");
}

String Window::Get_url() const {
	return resource_->Get_string("url");
}

END_UPP_NAMESPACE