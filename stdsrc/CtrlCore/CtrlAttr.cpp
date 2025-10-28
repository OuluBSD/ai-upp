// STL-backed CtrlCore API implementation

#include "CtrlAttr.h"

namespace Upp {

// Check if attribute exists
bool CtrlAttr::HasAttr(const String& name) const {
    return attributes.find(name) != attributes.end();
}

// Remove attribute
CtrlAttr& CtrlAttr::RemoveAttr(const String& name) {
    attributes.erase(name);
    return *this;
}

// Clear all attributes
CtrlAttr& CtrlAttr::ClearAttrs() {
    attributes.clear();
    return *this;
}

// Set control's style using attributes
CtrlAttr& CtrlAttr::SetStyle(dword style) {
    return Attr<String>("style", AsString(style));
}

dword CtrlAttr::GetStyle() const {
    String styleStr = GetAttr<String>("style", "0");
    return (dword)ScanInt(styleStr);
}

// Set control's data
CtrlAttr& CtrlAttr::SetData(const Value& data) {
    return Attr<Value>("data", data);
}

Value CtrlAttr::GetData() const {
    return GetAttr<Value>("data", Value());
}

// Set control's tag
CtrlAttr& CtrlAttr::SetTag(const Value& tag) {
    return Attr<Value>("tag", tag);
}

Value CtrlAttr::GetTag() const {
    return GetAttr<Value>("tag", Value());
}

// Set control's display
CtrlAttr& CtrlAttr::SetDisplay(const Display& display) {
    return Attr<const Display*>("display", &display);
}

const Display* CtrlAttr::GetDisplay() const {
    return GetAttr<const Display*>("display", nullptr);
}

}