#include "WebDriver.h"

NAMESPACE_UPP

Client::Client(const String& url) {
	// Initialize client with the given URL
	resource_ = detail::Shared<detail::Resource>(new detail::Resource(url, detail::Shared<detail::IHttp_client>(new detail::Http_client)));
}

Value Client::Get_status() const {
	// Return server status
	Value status = resource_->Get("status");
	// Already in Upp::Value format
	return status;
}

Vector<Session> Client::Get_sessions() const {
	// Return existing sessions
	Value result = resource_->Get("sessions");
	ValueArray session_values = From_json<ValueArray>(result);
	Vector<Session> sessions;
	for(const auto& val : session_values) {
		Value id_val = val["id"];
		String session_id = From_json<String>(id_val);
		sessions.Add(Session(detail::Shared<detail::Resource>(
			new detail::Resource(resource_, session_id, detail::Resource::Is_observer))));
	}
	return sessions;
}

Session Client::Create_session(
	const Capabilities& desired,
	const Capabilities& required
	) const {
	ValueMap caps;
	caps.Add("desiredCapabilities", To_json(desired));
	caps.Add("requiredCapabilities", To_json(required));

	Value response = resource_->Post("session", caps);
	String session_id = From_json<String>(response["sessionId"]);

	return Make_session(session_id, detail::Resource::Is_owner);
}

Session Client::Make_session(
	const String& id,
	detail::Resource::Ownership mode
	) const {
	return Session(detail::Shared<detail::Resource>(
		new detail::Resource(resource_, "session/" + id, mode)));
}

END_UPP_NAMESPACE