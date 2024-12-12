#include "Core.h"

NAMESPACE_UPP


#if 0
void LeadDataTemplate::Load() {
	String& dir = MetaDatabase::Single().dir;
	ASSERT(dir.GetCount());
	LoadFromJsonFileStandard(*this, dir + DIR_SEPS + "share-common" + DIR_SEPS + "template_lead_data.json");
}

void LeadDataTemplate::Store() {
	String& dir = MetaDatabase::Single().dir;
	StoreAsJsonFileStandard(*this, dir + DIR_SEPS + "share-common" + DIR_SEPS + "template_lead_data.json", true);
}
#endif

void LeadDataTemplate::Jsonize(JsonIO& json) {
	json		("templates", templates)
				("author_classes", author_classes)
				("author_specialities", author_specialities)
				("profit_reasons", profit_reasons)
				("organizational_reasons", organizational_reasons)
				("publishers", publishers)
				;
}

INITIALIZER_COMPONENT(LeadDataTemplate)

END_UPP_NAMESPACE
