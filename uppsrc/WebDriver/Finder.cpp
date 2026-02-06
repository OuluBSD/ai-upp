#include "WebDriver.h"

NAMESPACE_UPP

namespace detail {

static String GetElementId(const Value& v) {
	if (v.Is<ValueMap>()) {
		const ValueMap& m = v.Get<ValueMap>();
		int q = m.Find("element-6066-11e4-a52e-4f735466cecf");
		if (q >= 0) return m.GetValue(q);
		q = m.Find("ELEMENT");
		if (q >= 0) return m.GetValue(q);
	}
	return String();
}

Finder::Finder(
	const Shared<Resource>& resource,
	const Shared<IFinder_factory>& factory
	)
	: resource_(resource)
	, factory_(factory)
{
}

Element Finder::Find_element(const By& by) const {
	Value response = resource_->Post("element", To_json(by));
	String id = GetElementId(response);
	return Element(id, Make_sub_resource(resource_, "element/" + id), Shared<IFinder_factory>(new Element_factory()));
}

Vector<Element> Finder::Find_elements(const By& by) const {
	Value response = resource_->Post("elements", To_json(by));
	Vector<Element> elements;
	if (IsValueArray(response)) {
		const ValueArray& arr = response.Get<ValueArray>();
		for (const auto& v : arr) {
			String id = GetElementId(v);
			elements.Add(Element(id, Make_sub_resource(resource_, "element/" + id), Shared<IFinder_factory>(new Element_factory())));
		}
	}
	return elements;
}

} // namespace detail

END_UPP_NAMESPACE