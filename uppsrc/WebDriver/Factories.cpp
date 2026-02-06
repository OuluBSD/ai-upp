#include "WebDriver.h"

NAMESPACE_UPP

namespace detail {

Shared<Resource> Element_factory::operator()(const Shared<Resource>& resource) const {
	return Make_sub_resource(resource, "element");
}

Shared<Resource> Session_factory::operator()(const Shared<Resource>& resource) const {
	return Make_sub_resource(resource, "session");
}

} // namespace detail

END_UPP_NAMESPACE