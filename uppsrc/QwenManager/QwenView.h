#ifndef _QwenManager_QwenView_h_
#define _QwenManager_QwenView_h_

#include <CtrlLib/CtrlLib.h>

class QwenProjectView : public Ctrl {
	PageCtrl page;
	TerminalCtrl term;

	// Chat interface components
	LineEdit user_input;
	Button send_btn;
	DocEdit conversation_history;
	VScrollBar conversation_scroll;

	// Status display
	Label status_label;

public:
	struct Entry : Ctrl {
		DocEdit doc;

		Entry();
		void SetDocText(bool view_only=true);
	};

	Array<Entry> entries;

	Ptr<QwenProject> prj;


public:
	typedef QwenProjectView CLASSNAME;
	QwenProjectView();

	void Data(); // update data of ctrl
	void RefreshConversation(); // refresh the conversation display
	void OnSend(); // handle send button click
	void UpdateConnectionStatus(); // update connection status display
};


#endif
