#include <HtmlGui/HtmlGui.h>

using namespace Upp;

class HtmlGui01 : public HtmlApp {
public:
	typedef HtmlGui01 CLASSNAME;
	HtmlGui01() {
		Title("HtmlGui01 - Smoke Test");
	}

	virtual void Init() override {
		String html_content;
		html_content << "<html>"
		             << "<head>"
		             << "<script>"
		             << "function onNativeClick() {"
		             << "    console.log(\"Native button clicked!\");"
		             << "    document.getElementById(\"status\").innerHTML = \"Native click detected at \" + new Date().toLocaleTimeString();"
		             << "}"
		             << "</script>"
		             << "</head>"
		             << "<body>"
		             << "    <h1>HtmlGui01 Smoke Test</h1>"
		             << "    <p id=\"status\">Waiting for interaction...</p>"
		             << "    <hr>"
		             << "    <widget type=\"button\" id=\"mybtn\" onclick=\"onNativeClick()\">Native Button</widget>"
		             << "</body>"
		             << "</html>";
		html.SetHtml(html_content, "index.html");
	}
};

GUI_APP_MAIN
{
	HtmlGui01 app;
	app.Init();
	app.Run();
}
