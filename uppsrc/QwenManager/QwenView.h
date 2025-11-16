#ifndef _QwenManager_QwenView_h_
#define _QwenManager_QwenView_h_


class QwenProjectView : public Splitter {
	PageCtrl page;
	TerminalCtrl term;
	
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
	
};


#endif
