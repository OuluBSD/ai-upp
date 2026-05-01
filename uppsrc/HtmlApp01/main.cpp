#include <HtmlGui/HtmlGui.h>

using namespace Upp;

class HtmlApp01 : public TopWindow {
	Layout::HtmlCtrl html;
	HtmlDocument     doc;

public:
	typedef HtmlApp01 CLASSNAME;
	HtmlApp01() : doc(&html) {
		Add(html.SizePos());
		html.SetDocument(&doc);
		html.SetHtml("<html><body><h1>HtmlGui Test</h1><widget type='button'>Native Button</widget></body></html>", "test.html");
	}
};

GUI_APP_MAIN
{
	HtmlApp01().Run();
}
