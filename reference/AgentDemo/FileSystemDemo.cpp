#include "AgentDemo.h"

NAMESPACE_UPP

#if 0

1. Introduction

	Here’s a “File-System World” toy example that shows off creation, writing,
	moving and deletion of files and directories.

    Atoms (predicates)
		• dir_exists(Dir)
		• file_exists(F)
		• file_closed(F) (i.e. not open)
		• file_open(F)
		• file_mode(F,Mode) (Mode∈{read,write})
		• file_has_content(F,Content)

2. Actions

    CreateDir(D,Parent)
    Pre: dir_exists(Parent), ¬dir_exists(D)
    Post: dir_exists(D)

    CreateFile(F,Dir)
    Pre: dir_exists(Dir), ¬file_exists(F)
    Post: file_exists(F), file_closed(F), file_has_content(F,"")

    (Here F is a full path like “/home/admin/docs/readme.txt.”)

    OpenFile(F,Mode)
    Pre: file_exists(F), file_closed(F)
    Post: file_open(F), file_mode(F,Mode), ¬file_closed(F)

    WriteFile(F,Content)
    Pre: file_open(F), file_mode(F,write)
    Post: file_has_content(F,Content)

    CloseFile(F)
    Pre: file_open(F)
    Post: file_closed(F), ¬file_open(F), ¬file_mode(F,read), ¬file_mode(F,write)

    MoveFile(F,OldDir,NewDir)
    Pre: file_exists(OldPath), dir_exists(NewDir), file_closed(OldPath)
    where OldPath = OldDir / Name and NewPath = NewDir / Name
    Post: ¬file_exists(OldPath), file_exists(NewPath)

    DeleteDir(D)
    Pre: dir_exists(D)
    Post: ¬dir_exists(D)

3. Initial WorldState
	
	dir_exists("/")
	dir_exists("/home")
	dir_exists("/home/admin")
	(No files exist yet; no file_open or file_has_content atoms present.)

4. Goal
	
	file_exists("/home/readme.txt")
	file_has_content("/home/readme.txt","Hello")
	¬dir_exists("/home/admin/docs")

5. Story
	
	“As the sys-admin’s planner, you start with only “/”, “/home” and “/home/admin” in place.
	Your task is:
	
	    Create a subfolder “docs” under “/home/admin.”
	    Inside it, create and open a file “readme.txt,” write the text “Hello,” then close it.
	    Move that file up into “/home,” and finally delete the now-empty “docs” folder.
		By the end you must have “/home/readme.txt” containing “Hello,” and no more “/home/admin/docs.””
	
	This mini-domain exercises directory/file creation, opening, writing, moving and deletion—all via clear pre- and post-conditions.

#endif


void FileSystemExample() {
	using namespace UPP;
	VfsValue& app_root = MetaEnv().root;
	SearcherExt& searcher = app_root.GetAdd<SearcherExt>("searcher");
	searcher.SetVerbose();
	
	VfsValue& fs			= searcher.GetFS();
	CommitTreeExt& tree		= searcher.GetCommitTree();
	CommitDiffListExt& list	= searcher.GetCommitDiffList();
	
	ValueMap atoms;
	atoms.Add("armed with claws", true);
	atoms.Add("mouse visible",    false);
	atoms.Add("near mouse",       false);
	atoms.Add("at high place",    true);
	atoms.Add("claws extended",   false);
	atoms.Add("ready to attack",  false);
	atoms.Add("mouse alive",      true);
	atoms.Add("alive",            true);
	
	ValueMap goal;
	goal.Add("mouse alive",		  false);
	goal.Add("alive",			  true); // add this to avoid hurting by 'fall' action in the plan.
	goal.Add("near mouse",		  false);
	
	ValueMap actions;
	actions.Add("cat", ActionEventValue()
		.Pre("armed with claws", true)
		.Post("mouse visible", true));
	actions.Add("approach", ActionEventValue()
		.Pre("mouse visible", true)
		.Post("near mouse", true));
	actions.Add("come down", ActionEventValue()
		.Pre("at high place", true)
		.Post("at high place", false));
	actions.Add("aim", ActionEventValue()
		.Pre("mouse visible", true)
		.Pre("claws extended", true)
		.Post("ready to attack", true));
	actions.Add("attack", ActionEventValue()
		.Pre("ready to attack", true)
		.Pre("at high place", false)
		.Pre("near mouse", true)
		.Post("mouse alive", false));
	actions.Add("prepare claws", ActionEventValue()
		.Pre("armed with claws", true)
		.Post("claws extended", true));
	actions.Add("very high jump attack", ActionEventValue()
		.Pre("at high place", true)
		.Pre("near mouse", true)
		.Post("alive", false)
		.Post("mouse alive", false)
		.Cost(5));
	actions.Add("flee", ActionEventValue()
		.Pre("mouse visible", true)
		.Post("near mouse", false));
	
	ValueMap params;
	params("atoms") = atoms;
	params("actions") = actions;
	params("goal") = goal;
	params("dump_intermediate_trees") = false;
	searcher.SetSearcherParams(params);
	searcher.SetGeneratorParams(params, Null);
	searcher.SetSearchStrategy(SEARCHSTRATEGY_ASTAR);
	searcher.SetTerminalTest(TERMTEST_ACTION_PLANNER);
	searcher.SetGenerator(GENERATOR_ACTION_PLANNER);
	searcher.SetHeuristics(HEURISTIC_ACTION_PLANNER);
	searcher.ClearFS();
	searcher.WhenGenerated.Clear();
	searcher.WhenError = [](String s) {LOG("ActionPlanner error: " << s);};
	bool ret = searcher.RunSearch();
	if (!ret) {
		LOG(searcher.GetFS().GetTreeString());
	}
	ASSERT(ret);
	LOG(searcher.GetResultString());
}


END_UPP_NAMESPACE
