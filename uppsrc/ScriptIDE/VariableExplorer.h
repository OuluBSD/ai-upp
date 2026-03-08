#ifndef _ScriptIDE_VariableExplorer_h_
#define _ScriptIDE_VariableExplorer_h_

class VariableExplorer : public ArrayCtrl {
public:
    typedef VariableExplorer CLASSNAME;

    VariableExplorer();

    void SetVariables(const VectorMap<PyValue, PyValue>& vars);

private:
    struct VarDisplay : Display {
        virtual void Paint(Draw& w, const Rect& r, const Value& q,
                          Color ink, Color paper, dword style) const override;
    };

    static Color TypeColor(const String& type);
};

#endif
