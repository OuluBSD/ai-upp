#ifndef _WebDriver_Session_h_
#define _WebDriver_Session_h_

#include <Core/Core.h>

NAMESPACE_UPP

class Client;

class Session { // copyable
public:
	Capabilities Get_capabilities() const;
	String Get_source() const;
	String Get_title() const;
	String Get_url() const;
	String Get_screenshot() const; // Base64 PNG

	const Session& Navigate(const String& url) const;
	const Session& Get(const String& url) const; // Same as Navigate
	const Session& Forward() const;
	const Session& Back() const;
	const Session& Refresh() const;

	const Session& Execute(const String& script, const Js_args& args = Js_args()) const;
	template<typename T>
	T Eval(const String& script, const Js_args& args = Js_args()) const;
	const Session& Execute_async(const String& script, const Js_args& args = Js_args()) const;
	template<typename T>
	T Eval_async(const String& script, const Js_args& args = Js_args()) const;

	const Session& Set_focus_to_frame(const Element& frame) const;
	const Session& Set_focus_to_frame(const String& id) const;
	const Session& Set_focus_to_frame(int number) const;
	const Session& Set_focus_to_default_frame() const;
	const Session& Set_focus_to_parent_frame() const;

	Vector<Window> Get_windows() const;
	Window Get_current_window() const;
	const Session& Close_current_window() const;
	const Session& Set_focus_to_window(const String& window_name) const;
	const Session& Set_focus_to_window(const Window& window) const;

	Element Get_active_element() const;

	Element Find_element(const By& by) const;
	Vector<Element> Find_elements(const By& by) const;

	Vector<Cookie> Get_cookies() const;
	const Session& Set_cookie(const Cookie& cookie) const;
	const Session& Delete_cookies() const;
	const Session& Delete_cookie(const String& name) const;

	String Get_alert_text() const;
	const Session& Send_keys_to_alert(const String& text) const;
	const Session& Accept_alert() const;
	const Session& Dismiss_alert() const;

	const Session& Send_keys(const String& keys) const;
	const Session& Send_keys(const Shortcut& shortcut) const;

	const Session& Move_to_top_left_of(const Element&, const Offset& = Offset()) const;
	const Session& Move_to_center_of(const Element&) const;
	const Session& Move_to(const Offset&) const;
	const Session& Click(mouse::Button = mouse::Left_button) const;
	const Session& Double_click() const;
	const Session& Button_down(mouse::Button = mouse::Left_button) const;
	const Session& Button_up(mouse::Button = mouse::Left_button) const;

	const Session& Set_timeout_ms(timeout::Type type, int milliseconds);
	const Session& Set_implicit_timeout_ms(int milliseconds);
	const Session& Set_async_script_timeout_ms(int milliseconds);

	void Delete_session() const; // No need to delete sessions created by WebDriver or Client
	virtual ~Session() {}

private:
	friend class Client; // Only Client can create Sessions

	explicit Session(const detail::Shared<detail::Resource>& resource);

	Window Make_window(const String& handle) const;
	detail::Keyboard Get_keyboard() const;
	template<typename T>
	void Internal_eval(const String& webdriver_command,
		const String& script, const Js_args& args,
		T& result) const;
	void Internal_eval(const String& webdriver_command,
		const String& script, const Js_args& args,
		Element& result) const;
	picojson::value Internal_eval_json_value(const String& command,
		const String& script, const Js_args& args) const;
	const Session& Internal_set_focus_to_frame(const picojson::value& id) const;
	const Session& Internal_move_to(const Element*, const Offset*) const;
	const Session& Internal_mouse_button_command(const char* command, mouse::Button button) const;

private:
	detail::Shared<detail::Resource> resource_;
	detail::Shared<detail::Session_factory> factory_;
};

END_UPP_NAMESPACE

#endif