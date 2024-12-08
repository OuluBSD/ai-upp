#ifndef _AI_Ctrl_Artist_h_
#define _AI_Ctrl_Artist_h_

NAMESPACE_UPP

class ToolEditor;


class ArtistInfoCtrl : public WithArtistInfo<ToolAppCtrl> {
	
	
public:
	typedef ArtistInfoCtrl CLASSNAME;
	ArtistInfoCtrl();
	
	void Data();
	void Clear();
	void OnValueChange();
	
	
};

END_UPP_NAMESPACE

#endif
