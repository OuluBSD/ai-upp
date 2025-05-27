#ifndef _Core2_Component_h_
#define _Core2_Component_h_


struct DatasetPtrs;
class Engine;


struct DatasetProvider {
	virtual void GetDataset(DatasetPtrs& p) const = 0;
};

struct Component :
	VfsValueExt,
	Destroyable,
	Enableable,
	DatasetProvider
{
	Component(VfsValue& owner) : VfsValueExt(owner) {}
	
	void GetDataset(DatasetPtrs& p) const override;
	Engine& GetEngine();
	
	
	template <class S> void AddToSystem();
	template <class S> void RemoveFromSystem();
	
};

using ComponentPtr = Ptr<Component>;


#endif
