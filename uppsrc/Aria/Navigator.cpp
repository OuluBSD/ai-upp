#include "Aria.h"

NAMESPACE_UPP

AriaNavigator::AriaNavigator() {
	throttle_delay = ScanDouble(GetEnv("ARIA_THROTTLE_DELAY"));
	randomize_delay = ToLower(GetEnv("ARIA_RANDOMIZE_DELAY")) == "true";
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
	SaveFile(GetSessionFilePath(), AsJSON(ValueMap()("browser", browser_name)));
}

ValueMap AriaNavigator::LoadSessionData(const String& browser_name) const {
	String path = GetSessionFilePath(browser_name);
	if (FileExists(path)) {
		Value v = ParseJSON(LoadFile(path));
		if (v.Is<ValueMap>()) return v;
	}
	return ValueMap();
}

String AriaNavigator::GetCurrentBrowser() const {
	String path = GetSessionFilePath();
	if (FileExists(path)) {
		Value v = ParseJSON(LoadFile(path));
		if (v.Is<ValueMap>()) {
			ValueMap m = v;
			return m["browser"];
		}
	}
	return "";
}

void AriaNavigator::EnsureDriverRunning(const String& browser_name) {
	int port = 4444; // Default port
	String exe = browser_name == "firefox" ? "geckodriver" : "chromedriver";
	
	// Use ps to check if running
	String ps_out = Sys("ps aux");
	if (ps_out.Find(exe) >= 0) {
		return; // Already running
	}
	
	GetAriaLogger("navigator").Info("WebDriver (" + exe + ") not detected. Attempting to start...");
	
	String cmd;
	if (browser_name == "firefox") {
		cmd = "geckodriver --host 127.0.0.1 --port " + AsString(port);
	} else if (browser_name == "chrome") {
		cmd = "chromedriver --port " + AsString(port);
	} else {
		return;
	}
	
	// Check if executable exists in path
	String path = GetFileOnPath(exe, GetEnv("PATH"));
	if (path.IsEmpty()) {
		GetAriaLogger("navigator").Error("Could not find " + exe + " in PATH.");
		return;
	}
	
	// Start driver in background. We use system() with & to make it persistent/non-blocking
	// but a better way would be LocalProcess if we wanted to manage it.
	// For CLI usage, letting it run in background is often preferred.
	system(cmd + " > /dev/null 2>&1 &");
	
	// Wait a bit for it to start
	for (int i = 0; i < 20; i++) {
		Sleep(500);
		if (Sys("ps aux").Find(exe) >= 0) {
			GetAriaLogger("navigator").Info(exe + " started successfully.");
			return;
		}
	}
	
	GetAriaLogger("navigator").Error("Failed to start " + exe + ".");
}

void AriaNavigator::StartSession(const String& browser_name, bool headless) {
	EnsureDriverRunning(browser_name);
	GetAriaLogger("navigator").Info("Starting browser session: " + browser_name);
	
	Firefox caps; // Use Firefox specifically for now if browser_name is firefox
	if (browser_name == "firefox") {
		caps.ApplyStealthSettings();
		if (headless) caps.options.AddArgument("--headless");
		
		// Use patched binary
		String patched_exe = detail::PatchFirefoxBinary();
		if (!patched_exe.IsEmpty()) {
			caps.options.binary = patched_exe;
		}
	} else {
		caps.browser_name = browser_name;
	}
	
	try {
		driver.Create(caps, K_DEFAULT_WEB_DRIVER_URL, detail::Resource::IS_OBSERVER);
		driver->ApplyStealthJS();
		
		ValueMap session_data;
		session_data.Set("session_id", driver->GetSessionId());
		session_data.Set("browser", browser_name);
		
		SaveSession(browser_name, session_data);
		GetAriaLogger("navigator").Info("Aria session started for " + browser_name);
	} catch (const Exc& e) {
		GetAriaLogger("navigator").Error("Error starting session: " + e);
	}
}

bool AriaNavigator::ConnectToSession(const String& browser_name) {
	String name = browser_name.IsEmpty() ? GetCurrentBrowser() : browser_name;
	if (name.IsEmpty()) return false;
	
	EnsureDriverRunning(name);
	
	ValueMap data = LoadSessionData(name);
	if (data.IsEmpty()) return false;
	
	String session_id = data["session_id"];
	
	try {
		// Try to reconnect using the session_id
		driver.Create(session_id); 
		GetAriaLogger("navigator").Info("Successfully reconnected to " + name + " session " + session_id);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("navigator").Error("Failed to connect to " + name + " session: " + e);
		return false;
	}
}

void AriaNavigator::CloseSession(const String& browser_name) {
	if (driver) {
		driver->DeleteSession();
		driver.Clear();
	}
	String name = browser_name.IsEmpty() ? GetCurrentBrowser() : browser_name;
	if (!name.IsEmpty()) {
		DeleteFile(GetSessionFilePath(name));
		if (GetCurrentBrowser() == name)
			DeleteFile(GetSessionFilePath());
	}
}

void AriaNavigator::Navigate(const String& url) {
	Throttle();
	if (!driver && !ConnectToSession()) {
		throw SessionError("No active session.");
	}
	GetAriaLogger("navigator").Info("Navigating to: " + url);
	driver->Navigate(url);
	GetAriaLogger("navigator").Info("Injecting stealth JS");
	driver->ApplyStealthJS();
}

ValueArray AriaNavigator::ListTabs() {
	if (!driver && !ConnectToSession()) return ValueArray();
	
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
	if (!driver && !ConnectToSession()) return false;
	
	Vector<Window> windows = driver->GetWindows();
	if (identifier.Is<int>()) {
		int idx = identifier;
		if (idx >= 0 && idx < windows.GetCount()) {
			driver->SetFocusToWindow(windows[idx]);
			return true;
		}
	} else if (IsString(identifier)) {
		String s = identifier;
		for (const auto& w : windows) {
			if (w.GetHandle() == s) {
				driver->SetFocusToWindow(w);
				return true;
			}
			driver->SetFocusToWindow(w);
			if (driver->GetTitle() == s || driver->GetTitle().Find(s) >= 0) return true;
		}
	}
	return false;
}

String AriaNavigator::GetPageContent() {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->GetSource();
}

void AriaNavigator::NewTab(const String& url) {
	Throttle();
	if (!driver && !ConnectToSession()) return;
	driver->Execute("window.open('" + url + "', '_blank');", JsArgs());
	Vector<Window> windows = driver->GetWindows();
	if (windows.GetCount() > 0)
		driver->SetFocusToWindow(windows.Top());
}

ValueArray AriaNavigator::ExtractLinks() {
	if (!driver && !ConnectToSession()) return ValueArray();
	
	ValueArray links;
	Vector<Element> elements = driver->FindElements(By::TagName("a"));
	for (int i = 0; i < min(elements.GetCount(), 100); i++) {
		try {
			if (elements[i].IsDisplayed()) {
				String text = TrimBoth(elements[i].GetText());
				if (text.IsEmpty()) text = elements[i].GetAttribute("aria-label");
				String href = elements[i].GetAttribute("href");
				if (!href.IsEmpty() && (!text.IsEmpty() || !href.IsEmpty())) {
					links.Add(ValueMap()("id", i)("text", text.Left(50))("url", href));
				}
			}
		} catch (...) {}
	}
	return links;
}

void AriaNavigator::NavigateWithPrompt(const String& prompt) {
	GetAriaLogger("navigator").Info("Navigating with prompt: " + prompt);
	if (!driver && !ConnectToSession()) return;
	
	if (!ai_provider) {
		// Fallback if no AI
		String search_url = "https://duckduckgo.com/?q=" + UrlEncode(prompt);
		Navigate(search_url);
		return;
	}
	
	ValueArray links = ExtractLinks();
	String context = "Current URL: " + driver->GetUrl() + "\n";
	context << "Page Title: " << driver->GetTitle() << "\n";
	context << "Available Links:\n" << AsJSON(links, true);
	
	String ai_instruction = "You are an autonomous web navigator. The user wants to: " + prompt + "\n";
	ai_instruction << "Based on the available links, choose the best URL to visit. If none are relevant, suggest a search query.\n";
	ai_instruction << "Return JSON: {\"action\": \"navigate\", \"url\": \"...\"} OR {\"action\": \"search\", \"query\": \"...\"}";
	
	String response = ai_provider->Generate(ai_instruction, context, "json");
	Value v = ParseJSON(response);
	
	if (v.Is<ValueMap>()) {
		ValueMap m = v;
		if (m["action"] == "navigate") {
			Navigate(m["url"]);
		} else if (m["action"] == "search") {
			String search_url = "https://duckduckgo.com/?q=" + UrlEncode(m["query"]);
			Navigate(search_url);
		}
	} else {
		// Fallback
		String search_url = "https://duckduckgo.com/?q=" + UrlEncode(prompt);
		Navigate(search_url);
	}
}

Element AriaNavigator::WaitForElement(const String& selector, const String& by, int timeout) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	By b(by, selector);
	if (WaitUntilMatches(wait::ElementPresent(b), *driver, timeout * 1000)) {
		return driver->FindElement(b);
	}
	
	throw BrowserError("Timed out waiting for element: " + selector);
}

Vector<String> AriaNavigator::GetTabsByTag(const String& tag) {
	// Implementation for tab tagging would require persistent storage of tags per window handle
	return Vector<String>();
}

void AriaNavigator::TagTab(const Value& identifier, const String& tag) {
	// Implementation for tab tagging
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

END_UPP_NAMESPACE