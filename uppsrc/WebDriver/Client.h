#ifndef _WebDriver_Client_h_
#define _WebDriver_Client_h_

#include <Core/Core.h>

NAMESPACE_UPP

const char *const K_DEFAULT_WEB_DRIVER_URL = "http://localhost:4444/wd/hub/";

// Gives low level access to server's resources. You normally should not use it.
class Client { // copyable
public:
	explicit Client(const String& url = String(K_DEFAULT_WEB_DRIVER_URL));
	virtual ~Client() {}

	Value GetStatus() const;

	// Returns existing sessions.
	Vector<Session> GetSessions() const;

	// Creates new session.
	Session CreateSession(
		const Capabilities& desired,
		const Capabilities& required
		) const;

private:
	Session MakeSession(
		const String& id,
		detail::Resource::Ownership mode
		) const;

private:
	detail::Shared<detail::Resource> resource_;
};

END_UPP_NAMESPACE

#endif