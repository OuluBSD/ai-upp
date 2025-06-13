#include "AgentDemo.h"

NAMESPACE_UPP

#if 0

Here’s a “File-System World” toy example that shows off creation, writing, moving and deletion of files and directories.

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
	VfsValue& root			= MetaEnv().root;
	SearcherExt& modules	= root.GetAdd<SearcherExt>("searcher");
	VfsValue& fs			= modules.GetFS();
	CommitTreeExt& tree		= modules.GetCommitTree();
	CommitDiffListExt& list	= modules.GetCommitDiffList();
	
	modules.SetVerbose();
	modules.SetSearchStrategy(SEARCHSTRATEGY_ASTAR);
	modules.SetHeuristics(HEURISTIC_HAMMING_DISTANCE_OF_PREDICATES);
	
	bool ret = modules.RunSearch();
	ASSERT(ret);
	
	
}


END_UPP_NAMESPACE
