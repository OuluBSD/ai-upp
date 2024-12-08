#ifndef _AI_ImgCtrl_ImgAspectFixer_h_
#define _AI_ImgCtrl_ImgAspectFixer_h_

NAMESPACE_UPP

class ImageViewerCtrl : public Ctrl {
	Image img;
	
public:
	typedef ImageViewerCtrl CLASSNAME;
	ImageViewerCtrl();
	
	void Paint(Draw& d) override;
	void SetImage(const Image& i);
	void Clear();
	void Menu(Bar& menu);
	void RightDown(Point p, dword keyflags) override;

};

class AspectFixer;

class ImageAspectFixerTool : public ComponentCtrl {
	Splitter vsplit;
	WithImageAspectForm<Ctrl> form;
	ImageViewerCtrl from, to;
	String path;
	Image src_image;
	AspectFixer* af_ptr = 0;
	
	Vector<String> queue;
	
	void OpenFile(String path);
	
	Event<> WhenReady;
	
public:
	typedef ImageAspectFixerTool CLASSNAME;
	ImageAspectFixerTool();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void OnQueueSelect();
	void Do(int fn);
	void PostDo(int fn) {PostCallback(THISBACK1(Do, fn));}
	void QueueProcess(int cursor);
	
};

INITIALIZE(ImageAspectFixerTool)

END_UPP_NAMESPACE

#endif
