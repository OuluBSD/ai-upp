#include "WebDriver.h"

NAMESPACE_UPP

Client::Client(const String& url) {
	// Initialize client with the given URL
	resource_ = detail::Shared<detail::Resource>(new detail::Resource(url, /* http client */));
}

picojson::object Client::Get_status() const {
	// Return server status
	return resource_->Get("status").get<picojson::object>();
}

Vector<Session> Client::Get_sessions() const {
	// Return existing sessions
	picojson::value result = resource_->Get("sessions");
	Vector<picojson::object> session_objects = From_json<Vector<picojson::object>>(result);
	Vector<Session> sessions;
	for(const auto& obj : session_objects) {
		sessions.Add(Session(detail::Shared<detail::Resource>(
			new detail::Resource(resource_, obj.at("id").serialize(), detail::Resource::IsObserver))));
	}
	return sessions;
}

Session Client::Create_session(
	const Capabilities& desired,
	const Capabilities& required
	) const {
	JsonObject caps;
	caps.Set("desiredCapabilities", static_cast<const picojson::value&>(desired))
	   .Set("requiredCapabilities", static_cast<const picojson::value&>(required));

	picojson::value response = resource_->Post("session", caps);
	String session_id = From_json<String>(response.get("sessionId"));

	return Make_session(session_id, detail::Resource::IsOwner);
}

Session Client::Make_session(
	const String& id,
	detail::Resource::Ownership mode
	) const {
	return Session(detail::Shared<detail::Resource>(
		new detail::Resource(resource_, "session/" + id, mode)));
}

END_UPP_NAMESPACE