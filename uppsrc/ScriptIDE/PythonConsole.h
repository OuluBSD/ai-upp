#ifndef _ScriptIDE_PythonConsole_h_
#define _ScriptIDE_PythonConsole_h_

class PythonConsole : public DockableCtrl {
public:
    typedef PythonConsole CLASSNAME;
    PythonConsole();

    void Write(const String& s);
    void WriteError(const String& s);
    void Clear();

    String GetInput() const { return last_input; }
    Event<> WhenInput;
    Event<> WhenInterrupt;
    Event<> WhenRestart;
    Event<> WhenRemoveVariables;

    virtual bool Key(dword key, int count) override;

private:
	ToolBar toolbar;
    CodeEditor output;
    EditField input;
    String last_input;
    Vector<String> history;
    int history_index = -1;
    
    void OnInput();
    void LayoutToolbar(Bar& bar);
    void LayoutPaneMenu(Bar& bar);
};

#endif
