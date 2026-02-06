#ifndef _WebDriver_Resource_h_
#define _WebDriver_Resource_h_

#include <Core/Core.h>

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
		WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN()
		return From_json<T>(Get(command));
		WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(detail::Fmt() <<
			"command: " << command
			)
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
		WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN()
		Post(command, To_json(value));
		WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(detail::Fmt() <<
			"command: " << command
			)
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
		WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN()
		return Process_response((http_client_->*member)(
			Concat_url(url_, command)
			));
		WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(Fmt()
			<< "request: " << request_type
			<< ", command: " << command
			<< ", resource: " << url_
			)
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
		WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN()
		return Process_response((http_client_->*member)(
			Concat_url(url_, command),
			To_upload_data(upload_data)
			));
		WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(Fmt()
			<< "request: " << request_type
			<< ", command: " << command
			<< ", resource: " << url_
			<< ", data: " << To_upload_data(upload_data)
			)
	}

	Value Process_response(
		const Http_response& http_response
		) const {
		WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN()
		WEBDRIVERXX_CHECK(
			http_response.http_code / 100 != 4 &&
			http_response.http_code != 501,
			"HTTP code indicates that request is invalid");

		Value response;
		String error_message;
		if (!ParseJSON(response, http_response.body)) {
			WEBDRIVERXX_THROW(Fmt() << "JSON parser error");
		}

		WEBDRIVERXX_CHECK(response.IsObject(), "Server response is not an object");
		WEBDRIVERXX_CHECK(response.Has("status"), "Server response has no member \"status\"");
		WEBDRIVERXX_CHECK(response["status"].IsNumber(), "Response status code is not a number");
		const auto status =
			static_cast<response_status_code::Value>(ToInt(response["status"]));
		WEBDRIVERXX_CHECK(response.Has("value"), "Server response has no member \"value\"");
		const Value& value = response["value"];

		if (http_response.http_code == 500) { // Internal server error
			WEBDRIVERXX_CHECK(value.IsObject(), "Server response has no member \"value\" or \"value\" is not an object");
			WEBDRIVERXX_CHECK(value.Has("message"), "Server response has no member \"value.message\"");
			WEBDRIVERXX_CHECK(value["message"].IsString(), "\"value.message\" is not a string");
			WEBDRIVERXX_THROW(Fmt() << "Server failed to execute command ("
				<< "message: " << AsString(value["message"])
				<< ", status: " << response_status_code::ToString(status)
				<< ", status_code: " << status
				<< ")"
				);
		}
		WEBDRIVERXX_CHECK(status == response_status_code::kSuccess, "Non-zero response status code");
		WEBDRIVERXX_CHECK(http_response.http_code == 200, "Unsupported HTTP code");

		return Transform_response(response);
		WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(Fmt()
			<< "HTTP code: " << http_response.http_code
			<< ", body: " << http_response.body.c_str()
			)
	}

	static
	String Concat_url(const String& a, const String& b, const char delim = '/') {
		auto result = a.empty() ? b : a;
		if (!a.empty() && !b.empty()) {
			if (result[result.length()-1] != delim)
				result += delim;
			result.append(b[0] == delim ? b.substr(1) : b);
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
		response.Swap(result);
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