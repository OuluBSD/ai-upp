#ifndef _WebDriver_Finder_h_
#define _WebDriver_Finder_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

class Finder {
public:
	Finder(
		const Shared<Resource>& resource,
		const Shared<IFinder_factory>& factory
		);

	Element Find_element(const By& by) const;
	Vector<Element> Find_elements(const By& by) const;

private:
	Shared<Resource> resource_;
	Shared<IFinder_factory> factory_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif