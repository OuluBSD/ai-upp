#include "Aria.h"

NAMESPACE_UPP

AriaNavigator::AriaNavigator() {
	String td = GetEnv("ARIA_THROTTLE_DELAY");
	throttle_delay = td.IsEmpty() ? 1.0 : ScanDouble(td);
	randomize_delay = ToLower(GetEnv("ARIA_RANDOMIZE_DELAY")) != "false";
}

AriaNavigator::~AriaNavigator() {
	try {
		if (driver) {
			GetAriaLogger("navigator").Info("AriaNavigator: Auto-closing session...");
			driver->DeleteSession();
		}
	} catch(...) {}
}

void AriaNavigator::Throttle() {
	if (throttle_delay > 0) {
		double delay = throttle_delay;
		if (randomize_delay) {
			delay = delay * (0.8 + (Random(400) / 1000.0));
		}
		GetAriaLogger("navigator").Info(Format("Throttling for %.2f s", delay));
		Sleep(int(delay * 1000));
	}
}

String AriaNavigator::GetSessionFilePath(const String& browser_name) const {
	String aria_dir = GetHomeDirFile(".aria");
	RealizeDirectory(aria_dir);
	if (!browser_name.IsEmpty())
		return AppendFileName(aria_dir, "aria_session_" + browser_name + ".json");
	return AppendFileName(aria_dir, "aria_session_current.json");
}

void AriaNavigator::SaveSession(const String& browser_name, const ValueMap& session_data) {
	SaveFile(GetSessionFilePath(browser_name), AsJSON(session_data));
}

ValueMap AriaNavigator::LoadSessionData(const String& browser_name) const {
	String data = LoadFile(GetSessionFilePath(browser_name));
	if (data.IsEmpty()) return ValueMap();
	return ParseJSON(data);
}

String AriaNavigator::GetCurrentBrowser() const {
	ValueMap data = LoadSessionData("");
	return data["browser"];
}

void AriaNavigator::EnsureDriverRunning(const String& browser_name) {
	int port = 4444; 
	String exe = browser_name == "firefox" ? "geckodriver" : "chromedriver";
	
	try {
		HttpRequest req("http://127.0.0.1:4444/status");
		req.Timeout(1000);
		String res = req.Execute();
		if (!res.IsEmpty()) {
			GetAriaLogger("navigator").Info("WebDriver (" + exe + ") already running and responding.");
			return; 
		}
	} catch(...) {}

	GetAriaLogger("navigator").Info("WebDriver (" + exe + ") not responding or not detected. Starting...");
	
	if (Sys("pgrep " + exe).GetCount() > 0) {
		GetAriaLogger("navigator").Warning("Cleaning up unresponsive " + exe);
		system("killall -9 " + exe + " 2>/dev/null");
		Sleep(500);
	}
	
	String cmd;
	if (browser_name == "firefox") {
		cmd = "geckodriver --host 127.0.0.1 --port " + AsString(port);
	} else if (browser_name == "chrome") {
		cmd = "chromedriver --port " + AsString(port);
	} else {
		return;
	}
	
	String path = GetFileOnPath(exe, GetEnv("PATH"));
	if (path.IsEmpty()) {
		GetAriaLogger("navigator").Error("Could not find " + exe + " in PATH.");
		return;
	}
	
	String log_file = GetHomeDirFile(".aria/" + exe + ".log");
	system("nohup " + cmd + " > " + log_file + " 2>&1 &");
	
	for (int i = 0; i < 20; i++) {
		Sleep(500);
		try {
			HttpRequest req("http://127.0.0.1:4444/status");
			req.Timeout(500);
			if (!req.Execute().IsEmpty()) {
				GetAriaLogger("navigator").Info(exe + " started successfully.");
				return;
			}
		} catch(...) {}
	}
	
	GetAriaLogger("navigator").Error("Failed to start " + exe + ".");
}

void AriaNavigator::StartSession(const String& browser_name, bool headless, bool use_profile) {
	if (driver) {
		try {
			driver->GetUrl();
			return; 
		} catch(...) {
			driver.Clear();
		}
	}

	GetAriaLogger("navigator").Info("Starting fresh browser session: " + browser_name + (headless ? " (headless)" : ""));
	
	EnsureDriverRunning(browser_name);
	
	try {
		if (browser_name == "firefox") {
			Firefox caps;
			caps.ApplyStealthSettings();
			if (headless) caps.options.AddArgument("--headless");
			
			driver = new WebDriver::Web_driver(caps);
			session_id = driver->GetSessionId();
			
			ValueMap sd;
			sd.Add("session_id", session_id);
			sd.Add("browser", browser_name);
			SaveSession(browser_name, sd);
			SaveSession("", sd); 
			
			GetAriaLogger("navigator").Info("Session started: " + session_id);
		}
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Error("Failed to start session: " + String(e.what()));
		throw;
	}
}

bool AriaNavigator::ConnectToSession(const String& browser_name) {
	ValueMap session_data = LoadSessionData(browser_name);
	String sid = session_data["session_id"];
	
	if (sid.IsEmpty()) return false;
	
	GetAriaLogger("navigator").Info("Attempting to connect to existing session: " + sid);
	
	EnsureDriverRunning(browser_name);
	
	try {
		driver = new WebDriver::Web_driver(sid);
		driver->GetUrl(); // Test connection
		session_id = sid;
		GetAriaLogger("navigator").Info("Successfully reconnected to " + browser_name + " session " + sid);
		return true;
	} catch (...) {
		GetAriaLogger("navigator").Warning("Failed to reconnect to session " + sid + ". It may have expired.");
		driver.Clear();
		return false;
	}
}

void AriaNavigator::CloseSession(const String& browser_name) {
	if (driver) {
		try {
			driver->DeleteSession();
		} catch(...) {}
		driver.Clear();
	}
	String path = GetSessionFilePath(browser_name);
	if (FileExists(path)) FileDelete(path);
	if (browser_name.IsEmpty()) { // If we closed the current one, clear current mapping
		path = GetSessionFilePath("");
		if (FileExists(path)) FileDelete(path);
	}
}

void AriaNavigator::Navigate(const String& url) {
	Throttle();
	if (!driver && !ConnectToSession("firefox")) { 
		StartSession("firefox", IsAutomation());
		if (!driver) throw SessionError("Could not connect to or start a session.");
	}
	GetAriaLogger("navigator").Info("Navigating to: " + url);
	try {
		driver->ApplyStealthJS();
		driver->Get(url);
		driver->ApplyStealthJS();
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Error("Navigation failed: " + String(e.what()));
		if (ConnectToSession("firefox")) {
			driver->ApplyStealthJS();
			driver->Get(url);
			driver->ApplyStealthJS();
		} else {
			throw;
		}
	}
}

ValueArray AriaNavigator::ListTabs() {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	ValueArray tabs;
	Vector<Window> windows = driver->GetWindows();
	Window current = driver->GetCurrentWindow();
	
	for (int i = 0; i < windows.GetCount(); i++) {
		Window& w = windows[i];
		driver->SetFocusToWindow(w);
		tabs.Add(ValueMap()
			("index", i)
			("handle", w.GetHandle())
			("title", driver->GetTitle())
			("url", driver->GetUrl())
			("active", w.GetHandle() == current.GetHandle())
		);
	}
	
	driver->SetFocusToWindow(current);
	return tabs;
}

bool AriaNavigator::GotoTab(const Value& identifier) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	Vector<Window> windows = driver->GetWindows();
	if (identifier.Is<int>()) {
		int idx = identifier;
		if (idx >= 0 && idx < windows.GetCount()) {
			driver->SetFocusToWindow(windows[idx]);
			return true;
		}
	} else if (identifier.Is<String>()) {
		String target = identifier;
		for (const auto& w : windows) {
			driver->SetFocusToWindow(w);
			if (driver->GetTitle().Find(target) >= 0 || driver->GetUrl().Find(target) >= 0 || w.GetHandle() == target)
				return true;
		}
	}
	return false;
}

String AriaNavigator::GetPageContent() {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->GetSource();
}

void AriaNavigator::NewTab(const String& url) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	driver->Execute("window.open('" + url + "', '_blank');", JsArgs());
}

void AriaNavigator::NavigateWithPrompt(const String& prompt) {
	// AI-driven navigation logic
}

Element AriaNavigator::WaitForElement(const String& selector, const String& by, int timeout) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	TimeStop ts;
	while (ts.Elapsed() < timeout * 1000) {
		try {
			Element e = driver->FindElement(By(by, selector));
			if (e.IsDisplayed()) return e;
		} catch (...) {}
		Sleep(500);
	}
	
	throw BrowserError("Timed out waiting for element: " + selector);
}

Vector<String> AriaNavigator::GetTabsByTag(const String& tag) {
	return Vector<String>();
}

void AriaNavigator::TagTab(const Value& identifier, const String& tag) {
}

Value AriaNavigator::Eval(const String& script) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->Eval<Value>(script);
}

Element AriaNavigator::FindElement(const By& by) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->FindElement(by);
}

Vector<Element> AriaNavigator::FindElements(const By& by) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->FindElements(by);
}


String AriaNavigator::GetCurrentURL() {
        if (!driver && !ConnectToSession()) throw SessionError("No active session.");
        return driver->GetUrl();
}

END_UPP_NAMESPACE
