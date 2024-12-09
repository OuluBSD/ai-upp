#include "Core.h"

NAMESPACE_UPP

LeadDataTemplate::LeadDataTemplate() {
	
}

void LeadDataTemplate::Load() {
	String& dir = MetaDatabase::Single().dir;
	ASSERT(dir.GetCount());
	LoadFromJsonFileStandard(*this, dir + DIR_SEPS + "share-common" + DIR_SEPS + "template_lead_data.json");
}

void LeadDataTemplate::Store() {
	String& dir = MetaDatabase::Single().dir;
	StoreAsJsonFileStandard(*this, dir + DIR_SEPS + "share-common" + DIR_SEPS + "template_lead_data.json", true);
}

void LeadDataTemplate::Jsonize(JsonIO& json) {
	json		("templates", templates)
				("author_classes", author_classes)
				("author_specialities", author_specialities)
				("profit_reasons", profit_reasons)
				("organizational_reasons", organizational_reasons)
				("publishers", publishers)
				;
}

END_UPP_NAMESPACE
