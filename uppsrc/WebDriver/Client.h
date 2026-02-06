#ifndef _WebDriver_Client_h_
#define _WebDriver_Client_h_

#include <Core/Core.h>

NAMESPACE_UPP

const char *const k_default_web_driver_url = "http://localhost:4444/wd/hub/";

// Gives low level access to server's resources. You normally should not use it.
class Client { // copyable
public:
	explicit Client(const String& url = String(k_default_web_driver_url));
	virtual ~Client() {}

	Value Get_status() const;

	// Returns existing sessions.
	Vector<Session> Get_sessions() const;

	// Creates new session.
	Session Create_session(
		const Capabilities& desired,
		const Capabilities& required
		) const;

private:
	Session Make_session(
		const String& id,
		detail::Resource::Ownership mode
		) const;

private:
	detail::Shared<detail::Resource> resource_;
};

END_UPP_NAMESPACE

#endif