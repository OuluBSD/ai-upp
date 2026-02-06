#ifndef _WebDriver_WebDriver_h_
#define _WebDriver_WebDriver_h_

#include <Core/Core.h>
#include "Types.h"
#include "By.h"
#include "Capabilities.h"
#include "Client.h"
#include "Element.h"
#include "Errors.h"
#include "Session.h"
#include "Window.h"
#include "Chrome.h"
#include "Firefox.h"
#include "Ie.h"
#include "Phantom.h"
#include "Wait.h"
#include "WaitMatch.h"
#include <Core/Xmlize.h>

NAMESPACE_UPP

// Main WebDriver namespace for U++ implementation
namespace WebDriver {

// The main class for interactions with a server. Automatically connects to a server,
// creates and deletes a session and gives access to session's API.
class Web_driver : public Client, public Session {
public:
	explicit Web_driver(
		const Capabilities& desired = Capabilities(),
		const Capabilities& required = Capabilities(),
		const String& url = k_default_web_driver_url
		)
		: Client(url)
		, Session(Create_session(desired, required))
	{}
};

Web_driver Start(
	const Capabilities& desired,
	const Capabilities& required = Capabilities(),
	const String& url = k_default_web_driver_url
	);

Web_driver Start(const Capabilities& desired, const String& url);

} // namespace WebDriver

END_UPP_NAMESPACE

#endif