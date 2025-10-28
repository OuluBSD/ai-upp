#ifndef _CtrlCore_CtrlAttr_h_
#define _CtrlCore_CtrlAttr_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Display.h"
#include <functional>
#include <map>

// Control attributes and properties system
class CtrlAttr {
protected:
    Ctrl& ctrl;
    std::map<String, Value> attributes;
    
public:
    explicit CtrlAttr(Ctrl& c) : ctrl(c) {}
    
    // Set attribute with name and value
    template<typename T>
    CtrlAttr& Attr(const String& name, const T& value) {
        attributes[name] = value;
        return *this;
    }
    
    // Set multiple attributes from a map
    CtrlAttr& Attrs(const std::map<String, Value>& attrs) {
        for (const auto& pair : attrs) {
            attributes[pair.first] = pair.second;
        }
        return *this;
    }
    
    // Get attribute value
    template<typename T>
    T GetAttr(const String& name, const T& default_value) const {
        auto it = attributes.find(name);
        if (it != attributes.end()) {
            return it->second;
        }
        return default_value;
    }
    
    // Get attribute as a specific type
    Value GetAttr(const String& name) const {
        auto it = attributes.find(name);
        if (it != attributes.end()) {
            return it->second;
        }
        return Value();
    }
    
    // Check if attribute exists
    bool HasAttr(const String& name) const {
        return attributes.find(name) != attributes.end();
    }
    
    // Remove attribute
    CtrlAttr& RemoveAttr(const String& name) {
        attributes.erase(name);
        return *this;
    }
    
    // Clear all attributes
    CtrlAttr& ClearAttrs() {
        attributes.clear();
        return *this;
    }
    
    // Set control's style using attributes
    CtrlAttr& SetStyle(dword style) {
        return Attr("style", style);
    }
    
    dword GetStyle() const {
        return GetAttr<dword>("style", 0);
    }
    
    // Set control's data
    CtrlAttr& SetData(const Value& data) {
        return Attr("data", data);
    }
    
    Value GetData() const {
        return GetAttr<Value>("data", Value());
    }
    
    // Set control's tag
    CtrlAttr& SetTag(const Value& tag) {
        return Attr("tag", tag);
    }
    
    Value GetTag() const {
        return GetAttr<Value>("tag", Value());
    }
    
    // Set control's display
    CtrlAttr& SetDisplay(const Display& display) {
        return Attr("display", &display);
    }
    
    const Display* GetDisplay() const {
        const Display* ptr = GetAttr<const Display*>("display", nullptr);
        return ptr;
    }
};

// Attribute helper functions for building control properties
class Attrs {
public:
    template<typename T>
    static std::map<String, Value> Make(const String& name, const T& value) {
        std::map<String, Value> attrs;
        attrs[name] = value;
        return attrs;
    }
    
    template<typename T1, typename T2>
    static std::map<String, Value> Make(const String& name1, const T1& value1, 
                                       const String& name2, const T2& value2) {
        std::map<String, Value> attrs;
        attrs[name1] = value1;
        attrs[name2] = value2;
        return attrs;
    }
    
    template<typename T1, typename T2, typename T3>
    static std::map<String, Value> Make(const String& name1, const T1& value1, 
                                       const String& name2, const T2& value2,
                                       const String& name3, const T3& value3) {
        std::map<String, Value> attrs;
        attrs[name1] = value1;
        attrs[name2] = value2;
        attrs[name3] = value3;
        return attrs;
    }
    
    template<typename T1, typename T2, typename T3, typename T4>
    static std::map<String, Value> Make(const String& name1, const T1& value1, 
                                       const String& name2, const T2& value2,
                                       const String& name3, const T3& value3,
                                       const String& name4, const T4& value4) {
        std::map<String, Value> attrs;
        attrs[name1] = value1;
        attrs[name2] = value2;
        attrs[name3] = value3;
        attrs[name4] = value4;
        return attrs;
    }
};

// Fluent interface for setting control attributes
template<typename CtrlType>
class WithCtrlAttr {
private:
    CtrlType& ctrl;
    
public:
    explicit WithCtrlAttr(CtrlType& c) : ctrl(c) {}
    
    // Set common attributes
    WithCtrlAttr& Data(const Value& data) { ctrl.SetData(data); return *this; }
    WithCtrlAttr& Tag(const Value& tag) { ctrl.SetTag(tag); return *this; }
    WithCtrlAttr& Style(dword style) { ctrl.SetStyle(style); return *this; }
    WithCtrlAttr& Display(const Display& display) { ctrl.SetDisplay(display); return *this; }
    WithCtrlAttr& Label(const String& label) { ctrl.SetLabel(label); return *this; }
    WithCtrlAttr& Background(Color color) { ctrl.SetBackgroundColor(color); return *this; }
    WithCtrlAttr& ToolTip(const String& tip) { ctrl.SetToolTip(tip); return *this; }
    WithCtrlAttr& Enabled(bool enable = true) { ctrl.SetEnabled(enable); return *this; }
    WithCtrlAttr& Visible(bool visible = true) { ctrl.SetVisible(visible); return *this; }
    
    // For chaining
    CtrlType& operator()() { return ctrl; }
};

// Macro for convenient attribute setting
#define WITH_ATTR(ctrl) WithCtrlAttr(std::ref(ctrl))

// Template class for attribute-enabled controls
template<typename BaseCtrl>
class AttrCtrl : public BaseCtrl {
private:
    std::map<String, Value> attributes;
    
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    // Attribute interface
    template<typename T>
    AttrCtrl& Attr(const String& name, const T& value) {
        attributes[name] = value;
        return *this;
    }
    
    Value GetAttr(const String& name, const Value& default_value = Value()) const {
        auto it = attributes.find(name);
        return it != attributes.end() ? it->second : default_value;
    }
    
    bool HasAttr(const String& name) const {
        return attributes.find(name) != attributes.end();
    }
    
    // Common attribute shortcuts
    AttrCtrl& SetData(const Value& data) { return Attr("data", data); }
    Value GetData() const { return GetAttr("data"); }
    
    AttrCtrl& SetTag(const Value& tag) { return Attr("tag", tag); }
    Value GetTag() const { return GetAttr("tag"); }
    
    AttrCtrl& SetStyle(dword style) { return Attr("style", style); }
    dword GetStyle() const { return GetAttr("style", dword(0)); }
    
    AttrCtrl& SetDisplay(const Display& display) { return Attr("display", &display); }
    const Display* GetDisplay() const { 
        const Display* ptr = GetAttr("display", (const Display*)nullptr);
        return ptr;
    }
};

#endif