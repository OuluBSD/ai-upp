#ifndef _MaestroHub_IssueDialogs_h_
#define _MaestroHub_IssueDialogs_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class IssueEditDialog : public WithIssueEditLayout<TopWindow> {
	void SyncFromIssue(const MaestroIssue& src);
	void SyncToIssue(MaestroIssue& dst);
	
public:
	void LoadIssue(const MaestroIssue& src);
	bool RunEdit(MaestroIssue& dst);
	
	typedef IssueEditDialog CLASSNAME;
	IssueEditDialog();
};

class IssueCreateDialog : public WithIssueCreateLayout<TopWindow> {
public:
	MaestroIssue GetIssue();
	
	typedef IssueCreateDialog CLASSNAME;
	IssueCreateDialog();
};

class ListSelectDialog : public WithListSelectLayout<TopWindow> {
public:
	void SetChoices(const Vector<String>& choices);
	bool RunSelect(const String& title_text, const String& prompt_text, String& result);
	
	typedef ListSelectDialog CLASSNAME;
	ListSelectDialog();
};

bool CreateIssueTaskFile(const String& root, const MaestroIssue& issue, const String& title, String& out_path);

END_UPP_NAMESPACE

#endif
