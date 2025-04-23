#include "Core.h"

NAMESPACE_UPP


INITIALIZER_COMPONENT(LeadDataPublisher);

void LeadDataTemplate::Visit(Vis& v) {
	v.Ver(1)
	(1)	("templates", templates, 0)
		("author_classes", author_classes)
		("author_specialities", author_specialities)
		("profit_reasons", profit_reasons)
		("organizational_reasons", organizational_reasons)
		;
}

INITIALIZER_COMPONENT(LeadDataTemplate)

END_UPP_NAMESPACE
