#include "WebDriver.h"

NAMESPACE_UPP

Client::Client(const String& url) {
	// Initialize client with the given URL
	resource_ = detail::Shared<detail::Resource>(new detail::Resource(url, /* http client */));
}

Value Client::Get_status() const {
	// Return server status
	picojson::value status = resource_->Get("status");
	// Convert picojson::value to Upp::Value
	return ToValue(status);
}

Vector<Session> Client::Get_sessions() const {
	// Return existing sessions
	picojson::value result = resource_->Get("sessions");
	ValueArray session_values = From_json<ValueArray>(result);
	Vector<Session> sessions;
	for(const auto& val : session_values) {
		picojson::value id_val = val.get<picojson::object>().at("id");
		String session_id = From_json<String>(id_val);
		sessions.Add(Session(detail::Shared<detail::Resource>(
			new detail::Resource(resource_, session_id, detail::Resource::IsObserver))));
	}
	return sessions;
}

Session Client::Create_session(
	const Capabilities& desired,
	const Capabilities& required
	) const {
	picojson::object caps_obj;
	caps_obj["desiredCapabilities"] = To_json(desired);
	caps_obj["requiredCapabilities"] = To_json(required);
	picojson::value caps = picojson::value(caps_obj);

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