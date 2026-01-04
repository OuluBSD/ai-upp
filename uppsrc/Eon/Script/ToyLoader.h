#ifndef _Eon_Script_ToyLoader_h_
#define _Eon_Script_ToyLoader_h_


// ShaderToy file loader (libtopside .toy format == unpacked .js + stage info + separate glsl)

namespace Eon {



struct ToyInput : Moveable<ToyInput> {
	String				stage_name;
	String				id;
	String				type;
	String				filter;
	String				wrap;
	String				vflip;
	String				filename;
	String				generate_test_data;

	void Clear();

};

struct ToyStage : Moveable<ToyStage> {
	Vector<ToyInput>	inputs;
	Vector<String>		user_stages;
	String				name;
	String				type;
	String				output_id;
	String				script;
	String				script_path;
	String				stage_name;
	String				dump_screen_path; // Path for dumping screen content
	int					loopback_stage = -1;
	
};


class ToyLoader {
	Vector<ToyStage>	stages;
	Vector<String>		libraries;
	String				eon_script;
	
	
	bool FindStageNames();
	bool MakeScript();
	void PruneLibraries();
	bool SolveLoopbacks();
	
public:
	typedef ToyLoader CLASSNAME;
	ToyLoader();
	void UseRouterSyntax(bool enable = true) { use_router_syntax = enable; }

	bool Load(Value& o);

	String GetResult();

private:
	bool use_router_syntax = false;
	
	
	static ValueMap GetStageMap(int i, Value& o);
	static String GetStageType(int i, Value& o);
	static String GetStagePath(int i, Value& o);
	
};

}

#endif
