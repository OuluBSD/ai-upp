#ifndef _AI_VCS_h_
#define _AI_VCS_h_

NAMESPACE_UPP


class VersionControlSystem {
	bool storing = false;
	
public:
	typedef VersionControlSystem CLASSNAME;
	VersionControlSystem();
	~VersionControlSystem();
	void Initialize(String path);
	void Close();
	bool IsStoring() const;
	void SetStoring();
	void SetLoading();
};

END_UPP_NAMESPACE

#endif
