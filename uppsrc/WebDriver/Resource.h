#ifndef _WebDriver_Resource_h_
#define _WebDriver_Resource_h_

#include <Core/Core.h>
#include "ResponseStatusCode.h"

NAMESPACE_UPP

namespace detail {

class Resource : public Shared_object_base { // noncopyable
public:
	enum Ownership { Is_owner, Is_observer };

	Resource(
		const String& url,
		const Shared<IHttp_client>& http_client,
		Ownership mode = Is_observer
		)
		: http_client_(http_client)
		, url_(url)
		, ownership_(mode)
	{}

	Resource(
		const Shared<Resource>& parent,
		const String& name,
		Ownership mode = Is_observer
		)
		: http_client_(parent->http_client_)
		, parent_(parent)
		, url_(Concat_url(parent->url_, name))
		, ownership_(mode)
	{}

	virtual ~Resource() {
		try {
			if (ownership_ == Is_owner)
				Delete_resource();
		} catch (const std::exception&) {}
	}

	const String& Get_url() const {
		return url_;
	}

	Value Get(const String& command = String()) const {
		return Download(command, &IHttp_client::Get, "GET");
	}

	template<typename T>
	T Get_value(const String& command) const {
		try {
			return From_json<T>(Get(command));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: command: " + command);
		}
	}

	String Get_string(const String& command) const {
		return Get_value<String>(command);
	}

	bool Get_bool(const String& command) const {
		return Get_value<bool>(command);
	}

	Value Delete(const String& command = String()) const {
		return Download(command, &IHttp_client::Delete, "DELETE");
	}

	Value Post(
		const String& command = String(),
		const Value& upload_data = Value()
		) const {
		return Upload(command, upload_data, &IHttp_client::Post, "POST");
	}

	template<typename T>
	void Post(
		const String& command,
		const String& arg_name,
		const T& arg_value
		) const {
		ValueMap obj;
		obj.Add(arg_name, To_json(arg_value));
		Post(command, Value(obj));
	}

	template<typename T>
	void Post_value(const String& command, const T& value) const {
		try {
			Post(command, To_json(value));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: command: " + command);
		}
	}

protected:
	virtual Value Transform_response(Value& response) const {
		return response["value"];
	}

	virtual void Delete_resource() {
		Delete();
	}

private:
	Value Download(
		const String& command,
		Http_response (IHttp_client::* member)(const String& url) const,
		const char* request_type
		) const {
		try {
			return Process_response((http_client_.Get()->*member)(
				Concat_url(url_, command)
				));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "request: " + request_type
				+ ", command: " + command
				+ ", resource: " + url_
				);
		}
	}

	static String To_upload_data(const Value& upload_data)
	{
		return IsNull(upload_data) ? String() : AsJSON(upload_data);
	}

	Value Upload(
		const String& command,
		const Value& upload_data,
		Http_response (IHttp_client::* member)(const String& url, const String& upload_data) const,
		const char* request_type
		) const {
		try {
			return Process_response((http_client_.Get()->*member)(
				Concat_url(url_, command),
				To_upload_data(upload_data)
				));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "request: " + request_type
				+ ", command: " + command
				+ ", resource: " + url_
				+ ", data: " + To_upload_data(upload_data)
				);
		}
	}

	Value Process_response(
		const Http_response& http_response
		) const {
		try {
			if (!(http_response.http_code / 100 != 4 && http_response.http_code != 501)) {
				throw std::runtime_error("HTTP code indicates that request is invalid");
			}

			Value response = ParseJSON(http_response.body);
			if (response.IsError()) {
				throw std::runtime_error("JSON parser error");
			}

			if (!response.Is<ValueMap>()) throw std::runtime_error("Server response is not an object");
			if (response["status"].IsVoid()) throw std::runtime_error("Server response has no member \"status\"");
			if (!IsNumber(response["status"])) throw std::runtime_error("Response status code is not a number");
			const auto status =
				static_cast<response_status_code::Value>((int)response["status"]);
			if (response["value"].IsVoid()) throw std::runtime_error("Server response has no member \"value\"");
			const Value& value = response["value"];

			if (http_response.http_code == 500) { // Internal server error
				if (!value.Is<ValueMap>()) throw std::runtime_error("Server response has no member \"value\" or \"value\" is not an object");
				if (value["message"].IsVoid()) throw std::runtime_error("Server response has no member \"value.message\"");
				if (!value["message"].Is<String>()) throw std::runtime_error("\"value.message\" is not a string");
				throw std::runtime_error(String("Server failed to execute command (")
					+ "message: " + AsString(value["message"])
					+ ", status: " + response_status_code::ToString(status)
					+ ", status_code: " + AsString((int)status)
					+ ")"
				);
			}
			if (!(status == response_status_code::kSuccess)) throw std::runtime_error("Non-zero response status code");
			if (!(http_response.http_code == 200)) throw std::runtime_error("Unsupported HTTP code");

			return Transform_response(response);
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "HTTP code: " + AsString(http_response.http_code)
				+ ", body: " + http_response.body
				);
		}
	}

	static
	String Concat_url(const String& a, const String& b, const char delim = '/') {
		auto result = a.IsEmpty() ? b : a;
		if (!a.IsEmpty() && !b.IsEmpty()) {
			if (result[result.GetLength()-1] != delim)
				result.Cat(delim);
			result.Cat(b[0] == delim ? b.Mid(1) : b);
		}
		return result;
	}

private:
	const Shared<IHttp_client> http_client_;
	const Shared<Resource> parent_;
	const String url_;
	const Ownership ownership_;
};

class Root_resource : public Resource { // noncopyable
public:
	Root_resource(
		const String& url,
		const Shared<IHttp_client>& http_client
		)
		: Resource(url, http_client, Is_observer)
	{}

private:
	virtual Value Transform_response(Value& response) const {
		Value result;
		Swap(response, result);
		return result;
	}
};

inline
Shared<Resource> Make_sub_resource(
	const Shared<Resource>& parent,
	const String& subpath,
	Resource::Ownership mode = Resource::Is_observer
	) {
	return Shared<Resource>(new Resource(parent, subpath, mode));
}

inline
Shared<Resource> Make_sub_resource(
	const Shared<Resource>& parent,
	const String& subpath1,
	const String& subpath2,
	Resource::Ownership mode = Resource::Is_observer
	) {
	return Shared<Resource>(new Resource(parent, subpath1 + "/" + subpath2, mode));
}

} // namespace detail

END_UPP_NAMESPACE

#endif