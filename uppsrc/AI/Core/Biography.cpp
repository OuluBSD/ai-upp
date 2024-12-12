#include "Core.h"

NAMESPACE_UPP


void BioYear::RealizeImageSummaries() {
	if (images.IsEmpty()) return;
	int begin = 0;
	int last = images.GetCount()-1;
	
	for(int i = 1; i <= 5; i++) {
		int len = 1 << i;
		for(int j = 0; j < images.GetCount(); j += len) {
			int range_last = j + len - 1;
			if (i > 1 && range_last > last) // skip empty tail in largest range
				continue;
			BioImage& bi = GetAddImageSummary(j, len);
		}
		if (len >= images.GetCount())
			break;
	}
	SortByKey(image_summaries, BioRange());
}

BioImage& BioYear::GetAddImageSummary(int off, int len) {
	BioRange r;
	r.off = off;
	r.len = len;
	int i = image_summaries.Find(r);
	if (i >= 0)
		return image_summaries[i];
	BioImage& bi = image_summaries.Add(r);
	return bi;
}

int BioYear::FindElement(const String& key) const {
	int i = 0;
	for (const auto& e : elements) {
		if (e.key == key)
			return i;
		i++;
	}
	return -1;
}

double BioYear::Element::GetAverageScore() const {
	double score_sum = 0;
	int score_count = 0;
	for(int i = 0; i < SCORE_COUNT; i++) {
		int score = scores[i];
		score_sum += score;
		score_count++;
	}
	double score = score_sum / score_count;
	return score;
}

String BioYear::JoinElementMap(String delim0, String delim1) {
	String s;
	for (const BioYear::Element& e : elements) {
		s << e.key << delim0 << e.value << delim1;
	}
	return s;
}












void BiographyCategory::RealizeSummaries() {
	if (years.IsEmpty()) return;
	int begin_year = years[0].year;
	int last_year = years.Top().year;
	
	for(int i = 1; i <= 5; i++) {
		int len = 1 << i;
		for(int j = 0; j < years.GetCount(); j += len) {
			int range_last_year = years[j].year + len - 1;
			// When exceeding last year, allow only lowest range
			if (i > 1 && range_last_year > last_year)
				continue;
			BioYear& by = GetAddSummary(years[j].year, len);
		}
	}
	SortByKey(summaries, BioRange());
}

BioYear& BiographyCategory::GetAddSummary(int begin_year, int years) {
	BioRange r;
	r.off = begin_year;
	r.len = years;
	int i = summaries.Find(r);
	if (i >= 0)
		return summaries[i];
	BioYear& by = summaries.Add(r);
	by.year = begin_year + years - 1;
	return by;
}

int BiographyCategory::GetFilledCount() const {
	int c = 0;
	for (const BioYear& by : years)
		if (by.text.GetCount() || by.keywords.GetCount())
			c++;
	return c;
}

int BiographyCategory::GetFilledImagesCount() const {
	int c = 0;
	for (const BioYear& by : years)
		c += by.images.GetCount();
	return c;
}

BioYear& BiographyCategory::GetAdd(int year) {
	for (BioYear& by : years)
		if (by.year == year)
			return by;
	BioYear& by = years.Add();
	by.year = year;
	return by;
}












BiographyCategory& Biography::GetAdd(Owner& o, int enum_) {
	String s = GetBiographyCategoryEnum(enum_);
	BiographyCategory& bc = categories.GetAdd(s);
	int now = GetSysTime().year;
	if (o.year_of_birth > 0 && o.year_of_birth <= now) {
		for(int i = o.year_of_birth; i <= now; i++) {
			BioYear& by = bc.GetAdd(i);
		}
	}
	return bc;
}

BiographyCategory* Biography::Find(Owner& o, int enum_) {
	String s = GetBiographyCategoryEnum(enum_);
	int i = categories.Find(s);
	if (i < 0) return 0;
	return &categories[i];
}

const BiographyCategory* Biography::Find(Owner& o, int enum_) const {
	String s = GetBiographyCategoryEnum(enum_);
	int i = categories.Find(s);
	if (i < 0) return 0;
	return &categories[i];
}

void Biography::ClearSummary() {
	for (BiographyCategory& bc : categories) {
		bc.summaries.Clear();
		bc.RealizeSummaries();
	}
}
















void BiographyAnalysis::RealizePromptImageTypes() {
	for(int i = 0; i < image_types.GetCount(); i++)
		image_types[i].image_count = 0;
	
	for(int plat_i = 0; plat_i < platforms.GetCount(); plat_i++) {
		const PlatformBiographyAnalysis& pba = platforms[plat_i];
		for(int i = 0; i < pba.epk_photos.GetCount(); i++) {
			const PlatformAnalysisPhoto& pap = pba.epk_photos[i];
			String group = ToLower(pba.epk_photos.GetKey(i));
			PhotoPromptGroupAnalysis& ppga = image_types.GetAdd(group);
			ppga.image_count += 1; //pap.prompts.GetCount();
		}
	}
}

Vector<PhotoPromptLink> BiographyAnalysis::GetImageTypePrompts(String image_type) {
	Vector<PhotoPromptLink> o;
	image_type = ToLower(image_type);
	for(int plat_i = 0; plat_i < platforms.GetCount(); plat_i++) {
		PlatformBiographyAnalysis& pba = platforms[plat_i];
		int i = -1;
		for(int j = 0; j < pba.epk_photos.GetCount(); j++) {
			if (ToLower(pba.epk_photos.GetKey(j)) == image_type) {
				i = j;
				break;
			}
		}
		if (i >= 0) {
			PlatformAnalysisPhoto& pap = pba.epk_photos[i];
			//for(int j = 0; j < pap.prompts.GetCount(); j++) {
			//	PhotoPrompt& pp = pap.prompts[j];
			if (pap.prompts.GetCount()) {
				PhotoPrompt& pp = pap.prompts[0];
				PhotoPromptLink& ppl = o.Add();
				ppl.pap = &pap;
				ppl.pba = &pba;
				ppl.pp = &pp;
			}
		}
	}
	return o;
}

void BiographyAnalysis::Realize() {
	if (profiles.GetCount() < SOCIETYROLE_COUNT)
		profiles.SetCount(SOCIETYROLE_COUNT);
	for(int i = 0; i < SOCIETYROLE_COUNT; i++) {
		const auto& v = GetRoleProfile(i);
		profiles[i].SetCount(v.GetCount());
	}
	if (platforms.GetCount() < PLATFORM_COUNT)
		platforms.SetCount(PLATFORM_COUNT);
}

Index<int> BiographyAnalysis::GetRequiredRoles() const {
	Index<int> ret;
	TODO
	#if 0
	for(int role_i0 = 0; role_i0 < SOCIETYROLE_COUNT; role_i0++) {
		bool enabled = false;
		
		for(int i = 0; i < platforms.GetCount(); i++) {
			const PlatformBiographyAnalysis& pla = platforms[i];
			if (!pla.platform_enabled)
				continue;
			
			const Platform& plat = GetPlatforms()[i];
			const PlatformAnalysis& pa = mdb.GetAdd(plat);
			for (int role_i1 : pa.roles)
				if (role_i0 == role_i1)
					{enabled = true; break;}
			if (enabled) break;
		}
		
		if (enabled)
			ret.FindAdd(role_i0);
	}
	#endif
	return ret;
}

Index<int> BiographyAnalysis::GetRequiredCategories() const {
	Index<int> roles = GetRequiredRoles();
	Index<int> cats;
	for(int role_i : roles) {
		bool enabled = false;
		for (const auto& plat_roles : profiles) {
			for (const auto& plat_profs : plat_roles) {
				for (int cat_i : plat_profs.categories.GetKeys())
					cats.FindAdd(cat_i);
			}
		}
	}
	return cats;
}

INITIALIZER_COMPONENT(Biography)
INITIALIZER_COMPONENT(BiographySnapshot)

END_UPP_NAMESPACE
