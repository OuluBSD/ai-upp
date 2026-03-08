#include "ScriptIDE.h"

namespace Upp {

VariableExplorer::VariableExplorer()
{
    AddIndex("IDX");
    AddColumn("Name", 150).SetDisplay(StdDisplay());
    AddColumn("Type", 100).SetDisplay(StdDisplay());
    AddColumn("Size", 80).SetDisplay(StdRightDisplay());
    AddColumn("Value", 300).SetDisplay(Single<VarDisplay>());

    ColumnSort(1);  // Sort by Name (column 1)
    EvenRowColor();
}

void VariableExplorer::SetVariables(const VectorMap<PyValue, PyValue>& vars)
{
    Clear();

    int idx = 0;
    for(int i = 0; i < vars.GetCount(); i++) {
        const PyValue& key = vars.GetKey(i);
        const PyValue& val = vars[i];
        
        String name = key.ToString();
        String type = PyTypeName(val.GetType());
        String size;
        String value;

        // Calculate size based on type
        int type_kind = val.GetType();
        if(type_kind == PY_LIST || type_kind == PY_TUPLE || type_kind == PY_DICT || type_kind == PY_STR)
            size = AsString(val.GetCount());

        // Get string representation
        value = val.ToString();
        if(value.GetCount() > 100)
            value = value.Mid(0, 97) + "...";

        Add(idx++, name, type, size,
            AttrText(value).Paper(TypeColor(type)));
    }
}

Color VariableExplorer::TypeColor(const String& type)
{
    // Hash type name to hue (0-360)
    unsigned hash = 0;
    for(char c : type)
        hash = hash * 31 + (unsigned char)c;

    int hue = hash % 360;
    return HsvColorf(hue / 360.0, 0.15, 1.0);  // Light pastel colors
}

void VariableExplorer::VarDisplay::Paint(Draw& w, const Rect& r,
                                         const Value& q, Color ink,
                                         Color paper, dword style) const
{
    // Use AttrText paper color for type-based background
    if(IsType<AttrText>(q)) {
        const AttrText& at = ValueTo<AttrText>(q);
        w.DrawRect(r, at.paper);
        DrawSmartText(w, r.left + 2, r.top, r.Width() - 4, at.text.ToString(),
                      StdFont(), at.ink);
    } else {
        StdDisplay().Paint(w, r, q, ink, paper, style);
    }
}

}
