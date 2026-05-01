#include "DomBindings.h"

NAMESPACE_UPP

// Mock Element for document.getElementById
struct JsElement : JsUserData {
	String id;
	String innerHTML;
	
	virtual String GetTypeName() const override { return "Element"; }
	virtual JsValue GetAttr(const String& name) override {
		if(name == "id") return JsValue(id);
		if(name == "innerHTML") return JsValue(innerHTML);
		return JsValue::Undefined();
	}
	virtual bool SetAttr(const String& name, const JsValue& v) override {
		if(name == "innerHTML") {
			innerHTML = v.ToString();
			// In real implementation, this would trigger a DOM update
			return true;
		}
		return false;
	}
};

static JsValue document_getElementById(const Vector<JsValue>& args, void* user_data)
{
	if(args.GetCount() < 1) return JsValue::Undefined();
	String id = args[0].ToString();
	JsElement *el = new JsElement;
	el->id = id;
	return JsValue::UserData(el);
}

// WebSocket binding
static JsValue WebSocket_Ctor(const Vector<JsValue>& args, void* user_data)
{
	// In native mode, this will eventually be SimWebSocket
	return JsValue::Undefined();
}

static JsValue WebSocket_send(const Vector<JsValue>& args, void* user_data)
{
	// self.send(data)
	return JsValue::Undefined();
}

void InitDomBindings(JsVM& vm)
{
	JsValue& globals = vm.GetGlobalsRW();
	
	// document object
	JsValue document = JsValue::Object();
	RegisterFunction(document, "getElementById", document_getElementById);
	globals.SetItem(JsValue("document"), document);
	
	// console object
	JsValue console = JsValue::Object();
	RegisterFunction(console, "log", [](const Vector<JsValue>& args, void* user_data) -> JsValue {
		String s;
		for(int i = 0; i < args.GetCount(); i++) {
			if(i) s << " ";
			s << args[i].ToString();
		}
		RLOG(s);
		return JsValue::Undefined();
	});
	globals.SetItem(JsValue("console"), console);
	
	// WebSocket class
	JsValue ws_proto = JsValue::Object();
	RegisterFunction(ws_proto, "send", WebSocket_send);
	
	JsValue ws_class = JsValue::Function("WebSocket", WebSocket_Ctor);
	ws_class.SetItem(JsValue("prototype"), ws_proto);
	globals.SetItem(JsValue("WebSocket"), ws_class);
}

END_UPP_NAMESPACE
