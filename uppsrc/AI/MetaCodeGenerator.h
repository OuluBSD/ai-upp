#ifndef _AI_MetaCodeGenerator_h_
#define _AI_MetaCodeGenerator_h_

NAMESPACE_UPP

class MetaCodeGenerator {
	
public:
	struct File {
		String code;
		VectorMap<TextRange,Ptr<MetaNode>> range_nodes;
		VectorMap<TextRange,Ptr<MetaNode>> code_nodes;
	};
	
private:
	ArrayMap<PkgFile, File> files;
	
	
	
	void FindFiles(const MetaNodeSubset& n, Vector<Vector<int>>& pkgfiles);
	void FindNodes(const MetaNodeSubset& n, const PkgFile& key, Vector<const MetaNode*>& nodes);
	
public:
	typedef MetaCodeGenerator CLASSNAME;
	MetaCodeGenerator();
	
	bool Process(const MetaNodeSubset& n);
	String GetResult(int pkg, int file) const;
	const File* GetResultFile(int pkg, int file) const;
	
};

END_UPP_NAMESPACE

#endif
