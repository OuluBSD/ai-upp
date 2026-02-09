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
	
	// Check if already running and responding
	try {
		HttpRequest req("http://127.0.0.1:4444/status");
		req.Timeout(500);
		String res = req.Execute();
		if (!res.IsEmpty()) return; // Already running and healthy
	} catch(...) {}

	GetAriaLogger("navigator").Info("WebDriver (" + exe + ") not responding or not detected. Starting...");
	
	// Only kill if it was supposedly running but not responding
	if (Sys("ps aux").Find(exe) >= 0) {
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
	
	// Check if executable exists in path
	String path = GetFileOnPath(exe, GetEnv("PATH"));
	if (path.IsEmpty()) {
		GetAriaLogger("navigator").Error("Could not find " + exe + " in PATH.");
		return;
	}
	
	String log_file = GetHomeDirFile(".aria/" + exe + ".log");
	// Use nohup and setsid-like behavior to ensure it survives the parent
	system("nohup " + cmd + " > " + log_file + " 2>&1 &");
	
	// Wait a bit for it to start
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
	// If we are already connected to the right session, just return
	if (driver && GetCurrentBrowser() == browser_name) {
		try {
			driver->GetUrl();
			return; 
		} catch(...) {}
	}

	// Try to reconnect first if we use a profile
	if (use_profile && ConnectToSession(browser_name)) {
		return;
	}

	GetAriaLogger("navigator").Info("Starting fresh browser session: " + browser_name);
	
	// Do NOT call DeleteSession() here if we want persistence.
	// Just clear our local pointer.
	driver.Clear();
	
	EnsureDriverRunning(browser_name);
	
	Firefox caps; // Use Firefox specifically for now if browser_name is firefox
	if (browser_name == "firefox") {
		caps.ApplyStealthSettings();
		if (headless) caps.options.AddArgument("--headless");
		
		if (use_profile) {
			String profile_path = detail::GetFirefoxDefaultProfilePath();
			if (!profile_path.IsEmpty()) {
				GetAriaLogger("navigator").Info("Using Firefox profile: " + profile_path);
				caps.options.AddArgument("-profile");
				caps.options.AddArgument(profile_path);
			} else {
				GetAriaLogger("navigator").Warning("Could not find default Firefox profile.");
			}
		}
		
		// Use patched binary
		String patched_exe = detail::PatchFirefoxBinary();
		if (!patched_exe.IsEmpty()) {
			caps.options.binary = patched_exe;
		}
		
		if (!GetEnv("WAYLAND_DISPLAY").IsEmpty()) {
			caps.options.env.Set("MOZ_ENABLE_WAYLAND", "1");
		}
	} else {
		caps.browser_name = browser_name;
	}
	
	try {
		driver.Create(caps, K_DEFAULT_WEB_DRIVER_URL, detail::Resource::IS_OBSERVER);
		driver->ApplyStealthJS();
		
		ValueMap session_data;
		session_id = driver->GetSessionId();
		session_data.Set("session_id", session_id);
		session_data.Set("browser", browser_name);
		
		SaveSession(browser_name, session_data);
		GetAriaLogger("navigator").Info("Aria session started for " + browser_name);
	} catch (const std::exception& e) {
		String err = e.what();
		GetAriaLogger("navigator").Error("Error starting session: " + err);
		// If it failed due to profile lock, we might need to kill firefox once to take control,
		// but only as a last resort.
		if (err.Find("profile") >= 0 || err.Find("lock") >= 0) {
			GetAriaLogger("navigator").Warning("Profile lock detected. You might need to close other Firefox instances or we will have to kill it.");
		}
	}
}

bool AriaNavigator::ConnectToSession(const String& browser_name) {
	String name = browser_name.IsEmpty() ? GetCurrentBrowser() : browser_name;
	if (name.IsEmpty()) {
		return false;
	}
	
	EnsureDriverRunning(name);
	
	ValueMap data = LoadSessionData(name);
	if (data.IsEmpty()) {
		return false;
	}
	
	String sid = data["session_id"];
	
	try {
		// Try to reconnect using the session_id
		driver.Create(sid); 
		// Verify if session is still alive by getting URL
		driver->GetUrl();
		session_id = sid;
		GetAriaLogger("navigator").Info("Successfully reconnected to " + name + " session " + sid);
		return true;
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Info("Failed to reconnect to " + name + " session " + sid + ": " + String(e.what()));
		driver.Clear();
		return false;
	}
}

void AriaNavigator::CloseSession(const String& browser_name) {
	// We do NOT call driver->DeleteSession() because we want the browser to stay open.
	// We just clear our local reference.
	driver.Clear();
	
	String name = browser_name.IsEmpty() ? GetCurrentBrowser() : browser_name;
	// We also do NOT kill geckodriver or firefox here.
}

void AriaNavigator::Navigate(const String& url) {
	Throttle();
	if (!driver && !ConnectToSession("firefox")) { // Default to firefox if nothing else
		StartSession("firefox");
		if (!driver) throw SessionError("Could not connect to or start a session.");
	}
	GetAriaLogger("navigator").Info("Navigating to: " + url);
	// Use location.href trick to allow injection during load
	try {
		driver->ApplyStealthJS();
		driver->Execute("window.location.href = '" + url + "';", JsArgs());
		driver->ApplyStealthJS();
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Error("Navigation failed: " + String(e.what()));
		// If it failed because session died, try one more time after reconnecting
		if (ConnectToSession("firefox")) {
			driver->ApplyStealthJS();
			driver->Execute("window.location.href = '" + url + "';", JsArgs());
			driver->ApplyStealthJS();
		} else {
			throw;
		}
	}
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
	// Small fix: open about:blank first to inject stealth before target URL starts loading
	driver->Execute("window.open('about:blank', '_blank');", JsArgs());
	Vector<Window> windows = driver->GetWindows();
	if (windows.GetCount() > 0) {
		driver->SetFocusToWindow(windows.Top());
		driver->ApplyStealthJS();
		if (!url.IsEmpty() && url != "about:blank") {
			driver->Execute("window.location.href = '" + url + "';", JsArgs());
			driver->ApplyStealthJS();
		}
	}
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