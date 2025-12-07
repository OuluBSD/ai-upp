#include "IdeHost.h"
#ifdef flagGUI

namespace Upp {

static One<IdeHost> global_ide_host;

class HeadlessIde : public Ide {
public:
    HeadlessIde() {
        // Initialize in a headless/minimal GUI mode
        NoSeparator();
        NoIcon();
        NoCaption();
        // Don't show but still set up the basic IDE structure
    }
    
    // Override methods to prevent GUI operations in headless mode
    virtual void Show() override {
        // Don't show window in headless mode
    }
    
    virtual bool IsOpen() const override {
        // Consider it "open" for headless operations
        return true;
    }
    
    // Allow operations but disable GUI-specific features
    void HeadlessInit() {
        // Minimize GUI elements
        minimize = true;
        show_status_bar = false;
        toolbar_in_row = false;
        show_tabs = false;
    }
};

IdeHost::IdeHost() : initialized(false) {
}

IdeHost::~IdeHost() {
    ide.Clear();
}

bool IdeHost::InitializeLanguageAndCharset(String& error) {
    try {
        // Set up language and charset as TheIDE does
        int lng = SetLNGCharset(GetSystemLNG(), CHARSET_UTF8);
        if (FindIndex(CommandLine(), "--enus") >= 0)
            lng = LNG_enUS;
        SetLanguage(lng);
        SetDefaultCharset(CHARSET_UTF8);
        
        Ctrl::SetAppName("TheIDE-CLI");
        return true;
    }
    catch (...) {
        error = "Failed to initialize language and charset";
        return false;
    }
}

bool IdeHost::InitializeConfig(String& error) {
    try {
        // Set up basic configuration as TheIDE does
        ide->LoadConfig();
        ide->LoadAbbr();
        ide->HeadlessInit();
        return true;
    }
    catch (...) {
        error = "Failed to initialize IDE configuration";
        return false;
    }
}

bool IdeHost::InitializeIdeInstance(String& error) {
    try {
        // Create the IDE instance as headless
        ide.Set(new HeadlessIde());
        return ide != nullptr;
    }
    catch (...) {
        error = "Failed to create IDE instance";
        return false;
    }
}

bool IdeHost::Init(String& error) {
    if (initialized) {
        return true;
    }
    
    if (!InitializeIdeInstance(error)) {
        return false;
    }
    
    if (!InitializeLanguageAndCharset(error)) {
        return false;
    }
    
    if (!InitializeConfig(error)) {
        return false;
    }
    
    initialized = true;
    return true;
}

Ide& IdeHost::GetIde() {
    if (!initialized) {
        String error;
        if (!Init(error)) {
            // In a real implementation, we might want to handle this more gracefully
            // For now, we'll just return the IDE instance even if init partially failed
        }
    }
    return *ide;
}

const Ide& IdeHost::GetIde() const {
    if (!initialized) {
        // This is a const method, so we can't call Init
        // In practice, Init should be called before any usage
    }
    return *ide;
}

IdeHost& GetGlobalIdeHost() {
    if (!global_ide_host) {
        global_ide_host.Set(new IdeHost());
    }
    return *global_ide_host;
}

}

#endif // flagGUI