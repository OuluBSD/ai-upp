#ifndef _AI_Core_Profile_h_
#define _AI_Core_Profile_h_

NAMESPACE_UPP

struct Profile
{
	Owner* owner = 0;
	String name;
	Date begin;
	String biography;
	String preferred_genres;
	Index<int> languages;
	Array<BiographySnapshot> snapshots;
	
	BiographySnapshot* FindSnapshotRevision(int i);
	
	void Jsonize(JsonIO& json) {
		json
			("name", name)
			("begin", begin)
			("biography", biography)
			("preferred_genres", preferred_genres)
			("languages", languages)
			("snapshots", snapshots)
			;
		if (json.IsLoading() && snapshots.IsEmpty()) {
			auto& s = snapshots.Add();
			json
				("biography_detailed", s.data)
				("biography_analysis", s.analysis)
			;
			s.last_modified = GetSysTime();
		}
	}
	
};

END_UPP_NAMESPACE

#endif
