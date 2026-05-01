#ifndef _Vfs_Compiler_h_
#define _Vfs_Compiler_h_




struct Compiler :
	VfsValueExt
{
	String filepath, content;
	
	Tokenizer t;
	AstExporter ex;
	
public:
	CLASSTYPE(Compiler);
	Compiler(VfsValue& v);
	void Visit(Vis& v) override {}
	
	bool CompileEonFile(String filepath, ProgLang lang, String& output);
	bool CompileEon(String content, String path, ProgLang lang, String& output, bool verbose=false);
	AstNode* CompileAst(String content, String path, bool verbose=false);

	// Lighter entry point: tokenize + parse structure + semantic parse only.
	// Stops before AstRunner/meta. Returns the SemanticParser root, or null on error.
	AstNode* CompileToSemantic(const String& content, const String& path, bool verbose=false);
	
public:
	bool Tokenize(String filepath, String content, bool pythonic=false);
	bool ParseStructure();
	bool Parse();
	bool RunMeta();
	bool ExportHigh();
	bool ExportCpp();
	
	void OnProcMsg(ProcMsg msg);
	
	Callback1<ProcMsg> WhenMessage;
	
	template <class T> T& Get(String s) {return val.GetAdd<T>(s);}
	TokenStructure& GetTokenStructure() {return Get<TokenStructure>("TokenStructure");}
	SemanticParser& GetSemanticParser() {return Get<SemanticParser>("SemanticParser");}
	AstRunner& GetAstRunner() {return Get<AstRunner>("AstRunner");}
	
};



#endif
