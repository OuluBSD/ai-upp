#ifndef _AICore_Layer_h_
#define _AICore_Layer_h_

NAMESPACE_UPP

struct ImageLayer : Component {
	Image img;
	
	CLASSTYPE(ImageLayer)
	ImageLayer(MetaNode& owner) : Component(owner) {}
	~ImageLayer(){}
	String StoreString();
	void LoadString(const String& bz_enc);
	void Visit(Vis& v) override;
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_LAYER;}
};

INITIALIZE(ImageLayer);

struct TempImageLayer : Component {
	Vector<Image> imgs;
	
	CLASSTYPE(TempImageLayer)
	TempImageLayer(MetaNode& owner) : Component(owner) {}
	~TempImageLayer(){}
	void Visit(Vis& v) override {}
};

struct ImageGenLayer : TempImageLayer {
	CLASSTYPE(ImageGenLayer)
	ImageGenLayer(MetaNode& owner) : TempImageLayer(owner) {}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_GEN_LAYER;}
};

INITIALIZE(ImageGenLayer);

END_UPP_NAMESPACE

#endif
