#ifndef _Aria_Navigator_h_
#define _Aria_Navigator_h_

class BaseNavigator {
public:
	virtual ~BaseNavigator() {}
	virtual void StartSession(const String& browser_name = "chrome", bool headless = false) = 0;
	virtual bool ConnectToSession(const String& browser_name = "") = 0;
	virtual void CloseSession(const String& browser_name = "") = 0;
	virtual void Navigate(const String& url) = 0;
	virtual ValueArray ListTabs() = 0;
	virtual bool GotoTab(const Value& identifier) = 0;
	virtual String GetPageContent() = 0;
	virtual void NewTab(const String& url = "about:blank") = 0;
	virtual void NavigateWithPrompt(const String& prompt) = 0;
};

class AriaNavigator : public BaseNavigator {
	One<WebDriver::Web_driver> driver;
	double throttle_delay;
	bool randomize_delay;
	
	void Throttle();
	String GetSessionFilePath(const String& browser_name = "") const;
	void SaveSession(const String& browser_name, const ValueMap& session_data);
	ValueMap LoadSessionData(const String& browser_name) const;
	String GetCurrentBrowser() const;

public:
	AriaNavigator();
	
	virtual void StartSession(const String& browser_name = "chrome", bool headless = false) override;
	virtual bool ConnectToSession(const String& browser_name = "") override;
	virtual void CloseSession(const String& browser_name = "") override;
	virtual void Navigate(const String& url) override;
	virtual ValueArray ListTabs() override;
	virtual bool GotoTab(const Value& identifier) override;
	virtual String GetPageContent() override;
	virtual void NewTab(const String& url = "about:blank") override;
	virtual void NavigateWithPrompt(const String& prompt) override;
	
	Element WaitForElement(const String& selector, const String& by = "css selector", int timeout = 10);
	Vector<String> GetTabsByTag(const String& tag);
	void TagTab(const Value& identifier, const String& tag);
	ValueArray ExtractLinks();
};

#endif