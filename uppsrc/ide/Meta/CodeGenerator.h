#ifndef _Meta_CodeGenerator_h_
#define _Meta_CodeGenerator_h_


class MetaCodeGenerator {
	
public:
	struct File {
		String code;
		VectorMap<TextRange,Ptr<MetaNode>> range_nodes;
		VectorMap<TextRange,Ptr<MetaNode>> code_nodes;
		Vector<int> editor_to_line;
		Vector<MetaNode*> comment_to_node;
	};
	
private:
	ArrayMap<PkgFile, File> files;
	
	
	
	void FindFiles(const MetaNodeSubset& n, Vector<Vector<int>>& pkgfiles);
	void FindNodes(const MetaNodeSubset& n, const PkgFile& key, Vector<MetaNode*>& nodes);
	
public:
	typedef MetaCodeGenerator CLASSNAME;
	MetaCodeGenerator();
	
	bool Process(const MetaNodeSubset& n);
	String GetResult(int pkg, int file) const;
	const File* GetResultFile(int pkg, int file) const;
	
};


#endif
