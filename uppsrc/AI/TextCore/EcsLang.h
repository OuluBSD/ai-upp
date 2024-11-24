#ifndef _AI_TextCore_EcsLang_h_
#define _AI_TextCore_EcsLang_h_

NAMESPACE_UPP

class EcsIndexer : public IndexerExtension {
	int pkg_i = -1;
	int file_i = -1;
	
public:
	typedef EcsIndexer CLASSNAME;
	EcsIndexer();
	
	void RunJob(IndexerJob& job) override;
	bool RunCurrentFile() override;
	bool LoadEcsSpace(String path);
	bool MergeNode(MetaNode& root, EcsSpace& other);
	bool MergeVisit(Vector<MetaNode*>& scope, EcsSpace& other);
	static bool AcceptExt(String ext);
};

INITIALIZE(EcsIndexer)

END_UPP_NAMESPACE

#endif
