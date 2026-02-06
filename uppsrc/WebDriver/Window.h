#ifndef _WebDriver_Window_h_
#define _WebDriver_Window_h_

#include <Core/Core.h>


NAMESPACE_UPP

class Window { // copyable
public:
	Window();

	Window(
		const String& handle,
		const detail::Shared<detail::Resource>& resource
		);

	String Get_handle() const;

	Size Get_size() const;
	const Window& Set_size(const Size& size) const;

	Point Get_position() const;
	const Window& Set_position(const Point& position) const;

	const Window& Maximize() const;
	const Window& Fullscreen() const;

	const Window& Close() const;
	String Get_title() const;
	String Get_url() const;

private:
	String handle_;
	detail::Shared<detail::Resource> resource_;
};

END_UPP_NAMESPACE

#endif