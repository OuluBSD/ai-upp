#ifndef _AI_ImgCore_Layer_h_
#define _AI_ImgCore_Layer_h_

NAMESPACE_UPP

struct ImageLayer : Component {
	Image img;
	
	ImageLayer(MetaNode& owner) : Component(owner) {}
	~ImageLayer(){}
	String StoreString();
	void LoadString(const String& bz_enc);
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
	hash_t GetHashValue() const;
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_LAYER;}
};

INITIALIZE(ImageLayer);

struct TempImageLayer : Component {
	Vector<Image> imgs;
	
	TempImageLayer(MetaNode& owner) : Component(owner) {}
	~TempImageLayer(){}
	void Serialize(Stream& s) {} // do nothing
	void Jsonize(JsonIO& json) {}
	hash_t GetHashValue() const {return 0;}
};

struct ImageGenLayer : TempImageLayer {
	ImageGenLayer(MetaNode& owner) : TempImageLayer(owner) {}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_GEN_LAYER;}
};

INITIALIZE(ImageGenLayer);

END_UPP_NAMESPACE

#endif
