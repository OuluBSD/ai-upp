#ifndef _AI_ImgCore_Layer_h_
#define _AI_ImgCore_Layer_h_

NAMESPACE_UPP

struct ImageLayer : Component {
	Image img;
	
	ImageLayer(MetaNode& owner) : Component(owner) {}
	~ImageLayer(){}
	String StoreString();
	void LoadString(const String& bz_enc);
	void Visit(NodeVisitor& v) override;
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_LAYER;}
};

INITIALIZE(ImageLayer);

struct TempImageLayer : Component {
	Vector<Image> imgs;
	
	TempImageLayer(MetaNode& owner) : Component(owner) {}
	~TempImageLayer(){}
	void Visit(NodeVisitor& v) override {}
};

struct ImageGenLayer : TempImageLayer {
	ImageGenLayer(MetaNode& owner) : TempImageLayer(owner) {}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_IMG_GEN_LAYER;}
};

INITIALIZE(ImageGenLayer);

END_UPP_NAMESPACE

#endif
