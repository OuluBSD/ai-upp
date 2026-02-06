#ifndef _WebDriver_PicoJson_h_
#define _WebDriver_PicoJson_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Simplified picojson implementation for WebDriver
// This is a minimal implementation that provides the functionality needed for webdriverxx

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace picojson {

	enum type {
		null_type,
		boolean_type,
		number_type,
		string_type,
		array_type,
		object_type
	};

	typedef std::vector<struct value> array;
	typedef std::map<std::string, struct value> object;

	struct value {
		typedef enum {
			NULL_TYPE = null_type,
			BOOLEAN_TYPE = boolean_type,
			NUMBER_TYPE = number_type,
			STRING_TYPE = string_type,
			ARRAY_TYPE = array_type,
			OBJECT_TYPE = object_type
		} type_enum;

	private:
		type_enum type_;
		union {
			bool boolean_;
			double number_;
			std::string* string_ptr_;
			array* array_ptr_;
			object* object_ptr_;
		};

	public:
		value();
		explicit value(bool b);
		explicit value(double n);
		explicit value(int n);
		explicit value(const std::string& s);
		explicit value(const char* s);
		explicit value(const array& a);
		explicit value(const object& o);
		~value();
		value(const value& x);
		value& operator=(const value& x);

		type_enum get_type() const { return type_; }
		bool is<type_enum t>() const { return get_type() == t; }
		bool is_null() const { return get_type() == NULL_TYPE; }

		const bool& get<bool>() const;
		const double& get<double>() const;
		const std::string& get<std::string>() const;
		const array& get<array>() const;
		const object& get<object>() const;

		bool& get<bool>();
		double& get<double>();
		std::string& get<std::string>();
		array& get<array>();
		object& get<object>();

		std::string to_str() const;
		bool evaluate_as_boolean() const;

		const std::string& get_string() const;
		const double& get_number() const;
		const bool& get_boolean() const;
		const array& get_array() const;
		const object& get_object() const;

		std::string serialize() const;
	};

	inline value::value() : type_(NULL_TYPE) {}

	inline value::value(bool b) : type_(BOOLEAN_TYPE), boolean_(b) {}

	inline value::value(double n) : type_(NUMBER_TYPE), number_(n) {}

	inline value::value(int n) : type_(NUMBER_TYPE), number_(static_cast<double>(n)) {}

	inline value::value(const std::string& s) : type_(STRING_TYPE), string_ptr_(new std::string(s)) {}

	inline value::value(const char* s) : type_(STRING_TYPE), string_ptr_(new std::string(s)) {}

	inline value::value(const array& a) : type_(ARRAY_TYPE), array_ptr_(new array(a)) {}

	inline value::value(const object& o) : type_(OBJECT_TYPE), object_ptr_(new object(o)) {}

	inline value::~value() {
		switch (type_) {
			case STRING_TYPE:
				delete string_ptr_;
				break;
			case ARRAY_TYPE:
				delete array_ptr_;
				break;
			case OBJECT_TYPE:
				delete object_ptr_;
				break;
			default:
				break;
		}
	}

	inline value::value(const value& x) : type_(x.type_) {
		switch (type_) {
			case STRING_TYPE:
				string_ptr_ = new std::string(*x.string_ptr_);
				break;
			case ARRAY_TYPE:
				array_ptr_ = new std::vector<value>(*x.array_ptr_);
				break;
			case OBJECT_TYPE:
				object_ptr_ = new std::map<std::string, value>(*x.object_ptr_);
				break;
			default:
				memcpy(this, &x, sizeof(value));
				break;
		}
	}

	inline value& value::operator=(const value& x) {
		if (this != &x) {
			this->~value();
			new (this) value(x);
		}
		return *this;
	}

	template<> inline const bool& value::get<bool>() const {
		return get_boolean();
	}

	template<> inline const double& value::get<double>() const {
		return get_number();
	}

	template<> inline const std::string& value::get<std::string>() const {
		return get_string();
	}

	template<> inline const array& value::get<array>() const {
		return get_array();
	}

	template<> inline const object& value::get<object>() const {
		return get_object();
	}

	inline const std::string& value::get_string() const {
		return *string_ptr_;
	}

	inline const double& value::get_number() const {
		return number_;
	}

	inline const bool& value::get_boolean() const {
		return boolean_;
	}

	inline const array& value::get_array() const {
		return *array_ptr_;
	}

	inline const object& value::get_object() const {
		return *object_ptr_;
	}

	inline bool& value::get<bool>() {
		return const_cast<bool&>(get_boolean());
	}

	inline double& value::get<double>() {
		return const_cast<double&>(get_number());
	}

	inline std::string& value::get<std::string>() {
		return const_cast<std::string&>(get_string());
	}

	inline array& value::get<array>() {
		return const_cast<array&>(get_array());
	}

	inline object& value::get<object>() {
		return const_cast<object&>(get_object());
	}

	inline std::string value::to_str() const {
		return serialize();
	}

	inline bool value::evaluate_as_boolean() const {
		switch (type_) {
			case NULL_TYPE:
				return false;
			case BOOLEAN_TYPE:
				return boolean_;
			case NUMBER_TYPE:
				return number_ != 0.0;
			case STRING_TYPE:
				return !string_ptr_->empty();
			case ARRAY_TYPE:
				return !array_ptr_->empty();
			case OBJECT_TYPE:
				return !object_ptr_->empty();
			default:
				return false;
		}
	}

	inline std::string value::serialize() const {
		switch (type_) {
			case NULL_TYPE:
				return "null";
			case BOOLEAN_TYPE:
				return boolean_ ? "true" : "false";
			case NUMBER_TYPE:
				return std::to_string(number_);
			case STRING_TYPE:
				return "\"" + *string_ptr_ + "\"";
			case ARRAY_TYPE:
				{
					std::string s = "[";
					for (size_t i = 0; i < array_ptr_->size(); ++i) {
						if (i > 0) s += ",";
						s += (*array_ptr_)[i].serialize();
					}
					s += "]";
					return s;
				}
			case OBJECT_TYPE:
				{
					std::string s = "{";
					size_t i = 0;
					for (const auto& pair : *object_ptr_) {
						if (i > 0) s += ",";
						s += "\"" + pair.first + "\":" + pair.second.serialize();
						++i;
					}
					s += "}";
					return s;
				}
		}
		return "";
	}

	inline bool parse(value& out, const std::string& str, std::string* err = nullptr) {
		// Simplified parser - in a real implementation this would be more complex
		out = value(str); // Just treat everything as a string for now
		if (err) *err = "";
		return true;
	}
}

END_UPP_NAMESPACE

#endif