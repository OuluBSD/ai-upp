#ifndef _ide_WorkspaceService_h_
#define _ide_WorkspaceService_h_

class IdeCoreWorkspace {
public:
	bool LoadMainPackage(const String& main_package);

	const Workspace& GetWorkspace() const { return workspace; }
	const String&    GetMainPackage() const { return main_package; }

	Index<String> GetNests(bool refresh = false) const;

private:
	String    main_package;
	Workspace workspace;
};

#endif
