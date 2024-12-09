#include "Core.h"

NAMESPACE_UPP

LeadData::LeadData() {
	
}

void LeadData::Load() {
	String& dir = MetaDatabase::Single().dir;
	ASSERT(dir.GetCount());
	LoadFromJsonFileStandard(*this, dir + DIR_SEPS + "share" + DIR_SEPS + "lead_data.json");
}

void LeadData::Store() {
	String& dir = MetaDatabase::Single().dir;
	StoreAsJsonFileStandard(*this, dir + DIR_SEPS + "share" + DIR_SEPS + "lead_data.json", true);
}

void LeadData::Jsonize(JsonIO& json) {
	json("opportunities", opportunities);
}

LeadOpportunity& LeadData::GetAddOpportunity(int leadsite, String id) {
	for (LeadOpportunity& o : opportunities) {
		if (o.leadsite == leadsite && o.id == id) {
			Time now = GetSysTime();
			if (o.last_seen < now-12*60*60)
				o.last_seen = GetSysTime();
			return o;
		}
	}
	LeadOpportunity& o = opportunities.Add();
	o.first_seen = GetSysTime();
	o.last_seen = o.first_seen;
	o.leadsite = leadsite;
	o.id = id;
	return o;
}



#define OPP_LIST \
	ITEMV(links) \
	ITEMV(genres) \
	ITEMV(promoter_group_genres) \
	ITEMV(analyzed_booleans) \
	ITEMV(analyzed_string) \
	ITEMV(analyzed_lists) \
	ITEM(leadsite) \
	ITEM(id) \
	ITEM(first_seen) \
	ITEM(last_seen) \
	ITEM(name) \
	ITEM(band_opportunity_type) \
	ITEM(obj_class) \
	ITEM(request_entry_fee) \
	ITEM(request_featured) \
	ITEM(request_exclusive) \
	ITEM(request_curated) \
	ITEM(request_contest) \
	ITEM(request_comments) \
	ITEM(request_first_name) \
	ITEM(request_last_name) \
	ITEM(request_email) \
	ITEM(request_phone) \
	ITEM(request_description) \
	ITEM(request_opportunity_description) \
	ITEM(request_band_description) \
	ITEM(request_selection_description) \
	ITEM(vanity_url_id) \
	ITEM(vanity_url_name) \
	ITEM(status_text) \
	ITEM(description) \
	ITEM(band_opportunity_type_text) \
	ITEM(local_event_end_datetime) \
	ITEM(is_accepting_entries) \
	ITEM(deleted) \
	ITEM(address_str) \
	ITEM(public_image_url) \
	ITEM(logo_image_url) \
	ITEM(promoter_group_name) \
	ITEM(promoter_group_main_image_url) \
	ITEM(promoter_group_facebook_url) \
	ITEM(promoter_group_twitter_url) \
	ITEM(promoter_group_youtube_url) \
	ITEM(promoter_group_instagram_url) \
	ITEM(promoter_group_talent_description) \
	ITEM(promoter_group_short_description) \
	ITEM(promoter_group_talent_roster) \
	ITEM(promoter_group_opportunity_frequency_count) \
	ITEM(promoter_group_opportunity_frequency) \
	ITEM(compensated) \
	ITEM(min_compensation) \
	ITEM(max_compensation) \
	ITEM(pay_to_apply) \
	ITEM(free_to_apply) \
	ITEM(entry_count) \
	ITEM(min_entry_price_cents) \
	ITEM(entry_end_datetime) \
	ITEM(date_created) \
	ITEM(opp_score) \
	ITEM(money_score) \
	ITEM(money_score_rank) \
	ITEM(opp_score_rank) \
	ITEM(weighted_rank) \
	ITEM(chance_of_acceptance) \
	ITEM(average_payout_estimation) \
	ITEMV(chance_list) \
	ITEMV(typeclasses) \
	ITEMV(contents) \
	ITEMV(lyrics_ideas) \
	ITEMV(music_styles) \


void LeadOpportunity::Jsonize(JsonIO& json) {
	json
	#define ITEM(x) (#x, x)
	#define ITEMV(x) (#x, x)
	OPP_LIST
	;
	#undef ITEM
	#undef ITEMV
}

void LeadOpportunity::Genre::Jsonize(JsonIO& json) {
	json
		("id", id)
		("name", name)
		("primary", primary)
		;
}


int LeadOpportunity::GetCount() const {
	int c = 0;
	#define ITEM(x) ++c;
	#define ITEMV(x) ++c;
	OPP_LIST
	#undef ITEM
	#undef ITEMV
	return c;
}

Value LeadOpportunity::operator[](int i) const {
	int j = 0;
	#define ITEM(x) if (j++ == i) return x;
	#define ITEMV(x) j++;
	OPP_LIST
	#undef ITEM
	#undef ITEMV
	if (i == 0)
		return Join(links, ";");
	if (i == 1) {
		String s;
		for(const auto& g : genres) {
			if (!s.IsEmpty()) s << ", ";
			s << g.ToString();
		}
		return s;
	}
	if (i == 2) {
		String s;
		for(const auto& g : promoter_group_genres) {
			if (!s.IsEmpty()) s << ", ";
			s << g.ToString();
		}
		return s;
	}
	return Value();
}

const char* LeadOpportunity::GetKey(int i) const {
	int j = 0;
	#define ITEM(x) if (j++ == i) return #x;
	#define ITEMV(x) if (j++ == i) return #x;
	OPP_LIST
	#undef ITEM
	#undef ITEMV
	return "";
}


String LeadOpportunity::Genre::ToString() const {
	String s;
	s << id << ", " << name;
	if (primary) s << " (primary)";
	return s;
}

LeadDataAnalysis::LeadDataAnalysis() {
	
}

END_UPP_NAMESPACE
