#ifndef _AI_Ctrl_Artist_h_
#define _AI_Ctrl_Artist_h_

NAMESPACE_UPP


struct Artist : Component
{
	
	COMPONENT_CONSTRUCTOR(Artist)
	void Serialize(Stream& s) override {TODO}
	void Jsonize(JsonIO& json) override {TODO}
	hash_t GetHashValue() const override {TODO; return 0;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_ARTIST;}
	
};

INITIALIZE(Artist)

class ArtistInfoCtrl : public WithArtistInfo<ComponentCtrl> {
	
public:
	typedef ArtistInfoCtrl CLASSNAME;
	ArtistInfoCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override {}
	void Clear();
	void OnValueChange();
	
};

INITIALIZE(ArtistInfoCtrl)

END_UPP_NAMESPACE

#endif
