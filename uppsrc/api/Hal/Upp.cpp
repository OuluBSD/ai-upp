#include "Hal.h"

#if defined flagHAL && defined flagGUI

#include <Form/Form.hpp>

NAMESPACE_UPP


// EventsBase stub implementation (not used in GUI test)
struct HalUpp::NativeEventsBase {};

bool HalUpp::EventsBase_Create(NativeEventsBase*& dev) {
	dev = new NativeEventsBase;
	return true;
}

void HalUpp::EventsBase_Destroy(NativeEventsBase*& dev) {
	if (dev) {
		delete dev;
		dev = nullptr;
	}
}

bool HalUpp::EventsBase_Initialize(NativeEventsBase& dev, AtomBase& a, const WorldState& ws) {
	return true;
}

bool HalUpp::EventsBase_PostInitialize(NativeEventsBase& dev, AtomBase& a) {
	return true;
}

bool HalUpp::EventsBase_Start(NativeEventsBase& dev, AtomBase& a) {
	return true;
}

void HalUpp::EventsBase_Stop(NativeEventsBase& dev, AtomBase& a) {}

void HalUpp::EventsBase_Uninitialize(NativeEventsBase& dev, AtomBase& a) {}

bool HalUpp::EventsBase_Recv(NativeEventsBase& dev, AtomBase& a, int sink_ch, const Packet& in) {
	return true;
}

void HalUpp::EventsBase_Finalize(NativeEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg) {}

bool HalUpp::EventsBase_Send(NativeEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	return false;
}

void HalUpp::EventsBase_Visit(NativeEventsBase& dev, AtomBase& a, Visitor& vis) {}

void HalUpp::EventsBase_Update(NativeEventsBase& dev, AtomBase& a, double dt) {}

bool HalUpp::EventsBase_IsReady(NativeEventsBase& dev, AtomBase& a, PacketIO& io) {
	return false;
}

bool HalUpp::EventsBase_AttachContext(NativeEventsBase& dev, AtomBase& a, AtomBase& other) {
	return true;
}

void HalUpp::EventsBase_DetachContext(NativeEventsBase& dev, AtomBase& a, AtomBase& other) {}


// Mid-class between Atom and FormWindow
// Provides room for future Esc script VM, event I/O, and threading
struct HalUpp::NativeGuiSinkBase {
	// Form display (pointer to avoid early construction)
	FormWindow* form_win = nullptr;
	bool receipt_pending = false;

	// Event loop integration
	Ctrl::EventLoopContext event_ctx;
	bool event_ctx_active = false;

	// Future expansion points (not implemented yet):
	// - Esc::VM script_vm;              // Esc script virtual machine
	// - Vector<Thread> script_threads;  // Scripting thread pool
	// - PacketBuffer event_in;          // Incoming event packets
	// - PacketBuffer event_out;         // Outgoing event packets

	bool initialized = false;
};

// Device interface functions
bool HalUpp::GuiSinkBase_Create(NativeGuiSinkBase*& dev) {
	dev = new NativeGuiSinkBase;
	return true;
}

void HalUpp::GuiSinkBase_Destroy(NativeGuiSinkBase*& dev) {
	if (dev) {
		if (dev->form_win) {
			delete dev->form_win;
			dev->form_win = nullptr;
		}
		delete dev;
		dev = nullptr;
	}
}

bool HalUpp::GuiSinkBase_Initialize(NativeGuiSinkBase& dev, AtomBase& a, const WorldState& ws) {
	LOG("GuiSinkBase_Initialize: starting");

	// Create FormWindow now that GUI system is initialized
	dev.form_win = new FormWindow;
	LOG("GuiSinkBase_Initialize: FormWindow created");
	dev.form_win->Add(dev.form_win->GetForm().SizePos());

	// Setup window from configuration
	bool sizeable = ws.GetBool(".sizeable", true);
	bool close_machine = ws.GetBool(".close_machine", true);

	if (sizeable)
		dev.form_win->Sizeable();

	if (close_machine)
		dev.form_win->WhenClose = [&a]() { a.GetEngine().SetNotRunning(); };

	// Window will be sized by form layout
	dev.form_win->Title("GUI Event Viewer");

	dev.initialized = true;
	LOG("GuiSinkBase_Initialize: complete");
	return true;
}

bool HalUpp::GuiSinkBase_PostInitialize(NativeGuiSinkBase& dev, AtomBase& a) {
	return true;
}

bool HalUpp::GuiSinkBase_Start(NativeGuiSinkBase& dev, AtomBase& a) {
	return true;
}

void HalUpp::GuiSinkBase_Stop(NativeGuiSinkBase& dev, AtomBase& a) {
	a.ClearDependencies();
}

void HalUpp::GuiSinkBase_Uninitialize(NativeGuiSinkBase& dev, AtomBase& a) {
	LOG("GuiSinkBase_Uninitialize: starting");

	if (dev.event_ctx_active) {
		LOG("GuiSinkBase_Uninitialize: ending event loop");
		Ctrl::EventLoopEnd(dev.event_ctx);
		dev.event_ctx_active = false;
		LOG("GuiSinkBase_Uninitialize: event loop ended");
	}

	if (dev.form_win) {
		LOG("GuiSinkBase_Uninitialize: closing form window");
		dev.form_win->Close();
		LOG("GuiSinkBase_Uninitialize: form window closed");
	}

	dev.initialized = false;

	LOG("GuiSinkBase_Uninitialize: removing from update list");
	a.RemoveAtomFromUpdateList();
	LOG("GuiSinkBase_Uninitialize: complete");
}

bool HalUpp::GuiSinkBase_Recv(NativeGuiSinkBase& dev, AtomBase& a, int sink_ch, const Packet& in) {
	if (!dev.initialized)
		return false;

	ValueFormat fmt = in->GetFormat();
	if (!fmt.IsGui()) {
		LOG("UppGuiSinkDevice: expected GUI format, got " << fmt.ToString());
		return false;
	}

	// Extract GUI form data from packet
	// The packet data contains the form XML string
	const Vector<byte>& data = in->GetData();
	if (data.GetCount() <= 0) {
		LOG("UppGuiSinkDevice: empty packet data");
		return false;
	}

	String form_xml((const char*)data.Begin(), data.GetCount());

	if (!dev.form_win) {
		LOG("UppGuiSinkDevice: form_win not created yet");
		return false;
	}

	// Load form from XML
	LOG("UppGuiSinkDevice: Loading form from XML (" << form_xml.GetCount() << " bytes)");
	if (!dev.form_win->LoadString(form_xml, false)) {
		LOG("UppGuiSinkDevice: failed to load form from XML");
		return false;
	}

	// Align/valign values of 0 mean CENTER in U++, but older forms store 0 for LEFT/TOP.
	// If explicit offsets are present, treat 0 as LEFT/TOP to preserve expected layout.
	Form& form = dev.form_win->GetForm();
	for (FormLayout& layout : form.GetLayouts()) {
		Array<FormObject>& objects = layout.GetObjects();
		for (int i = 0; i < objects.GetCount(); ++i) {
			FormObject& obj = objects[i];
			const Rect& r = obj.GetRect();
			if (obj.GetHAlign() == Ctrl::CENTER && r.left != 0)
				obj.SetHAlign(Ctrl::LEFT);
			if (obj.GetVAlign() == Ctrl::CENTER && r.top != 0)
				obj.SetVAlign(Ctrl::TOP);
		}
	}

	// Select default layout
	String layout_name = "Default";
	if (!dev.form_win->Layout(layout_name)) {
		LOG("UppGuiSinkDevice: layout '" << layout_name << "' not found");
		return false;
	}

	// Open window if not visible
	if (!dev.form_win->IsOpen()) {
		dev.form_win->Open();
		LOG("UppGuiSinkDevice: window opened");
	}
	dev.receipt_pending = true;

	return true;
}

void HalUpp::GuiSinkBase_Finalize(NativeGuiSinkBase& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	if (!dev.initialized || !dev.form_win || !dev.form_win->IsOpen())
		return;

	// Process one event loop iteration
	if (!dev.event_ctx_active) {
		dev.event_ctx = Ctrl::EventLoopBegin(NULL);
		dev.event_ctx_active = true;
	}

	Ctrl::EventLoopIteration(dev.event_ctx);
}

bool HalUpp::GuiSinkBase_Send(NativeGuiSinkBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	if (!dev.receipt_pending)
		return false;

	// Send receipt acknowledging GUI display
	ValueFormat fmt;
	fmt.SetReceipt(DevCls::CENTER);
	out.SetFormat(fmt);

	// TODO: Set receipt data (timestamp, status, etc.)

	dev.receipt_pending = false;
	return true;
}

void HalUpp::GuiSinkBase_Visit(NativeGuiSinkBase& dev, AtomBase& a, Visitor& vis) {
	// Visit for serialization/inspection
}

void HalUpp::GuiSinkBase_Update(NativeGuiSinkBase& dev, AtomBase& a, double dt) {
	// Update for time-based operations
}

bool HalUpp::GuiSinkBase_IsReady(NativeGuiSinkBase& dev, AtomBase& a, PacketIO& io) {
	// Always ready to receive GUI data when initialized
	return dev.initialized;
}

bool HalUpp::GuiSinkBase_AttachContext(NativeGuiSinkBase& dev, AtomBase& a, AtomBase& other) {
	return true;
}

void HalUpp::GuiSinkBase_DetachContext(NativeGuiSinkBase& dev, AtomBase& a, AtomBase& other) {
	// Detach context
}


// GuiFileSrc implementation
struct HalUpp::NativeGuiFileSrc {
	String form_file_path;
	String form_xml_data;
	bool loaded = false;
	bool sent = false;
};

bool HalUpp::GuiFileSrc_Create(NativeGuiFileSrc*& dev) {
	dev = new NativeGuiFileSrc;
	return true;
}

void HalUpp::GuiFileSrc_Destroy(NativeGuiFileSrc*& dev) {
	if (dev) {
		delete dev;
		dev = nullptr;
	}
}

bool HalUpp::GuiFileSrc_Initialize(NativeGuiFileSrc& dev, AtomBase& a, const WorldState& ws) {
	LOG("GuiFileSrc_Initialize: starting");
	// Get file path from configuration
	dev.form_file_path = ws.GetString(".file", String());
	if (dev.form_file_path.IsEmpty()) {
		LOG("GuiFileSrc: no file path specified");
		return false;
	}

	// Load the .form file
	dev.form_xml_data = LoadFile(dev.form_file_path);
	if (dev.form_xml_data.IsEmpty()) {
		LOG("GuiFileSrc: failed to load file: " << dev.form_file_path);
		return false;
	}

	LOG("GuiFileSrc: loaded file " << dev.form_file_path << " (" << dev.form_xml_data.GetCount() << " bytes)");
	dev.loaded = true;
	dev.sent = false;
	return true;
}

bool HalUpp::GuiFileSrc_PostInitialize(NativeGuiFileSrc& dev, AtomBase& a) {
	return true;
}

bool HalUpp::GuiFileSrc_Start(NativeGuiFileSrc& dev, AtomBase& a) {
	return true;
}

void HalUpp::GuiFileSrc_Stop(NativeGuiFileSrc& dev, AtomBase& a) {
	a.ClearDependencies();
}

void HalUpp::GuiFileSrc_Uninitialize(NativeGuiFileSrc& dev, AtomBase& a) {
	dev.form_xml_data.Clear();
	dev.loaded = false;
	dev.sent = false;
	a.RemoveAtomFromUpdateList();
}

bool HalUpp::GuiFileSrc_Recv(NativeGuiFileSrc& dev, AtomBase& a, int sink_ch, const Packet& in) {
	// Receive orders to reload or send
	ValueFormat fmt = in->GetFormat();
	if (fmt.IsOrder()) {
		// Ignore orders; this atom only sends the form once
		LOG("GuiFileSrc: received order, ignoring (single-send)");
	}
	return true;
}

void HalUpp::GuiFileSrc_Finalize(NativeGuiFileSrc& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	// Nothing to finalize
}

bool HalUpp::GuiFileSrc_Send(NativeGuiFileSrc& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	if (!dev.loaded || dev.sent)
		return false;

	// Create GUI packet with form XML data
	ValueFormat fmt;
	fmt.SetGui(DevCls::CENTER);
	out.SetFormat(fmt);

	// Set packet data to the form XML
	int data_size = dev.form_xml_data.GetCount();
	Vector<byte>& data = out.Data();
	data.SetCount(data_size);
	memcpy(data.Begin(), dev.form_xml_data.Begin(), data_size);

	LOG("GuiFileSrc: sending GUI packet (" << data_size << " bytes)");
	if (a.packet_router && !a.router_source_ports.IsEmpty() && fmt.IsValid()) {
		int credits = a.RequestCredits(src_ch, 1);
		if (credits <= 0) {
			RTLOG("GuiFileSrc_Send: credit request denied for src_ch=" << src_ch);
			return false;
		}
		Packet route_pkt = CreatePacket(out.GetOffset());
		route_pkt->Pick(out);
		route_pkt->SetFormat(fmt);
		bool routed = a.EmitViaRouter(src_ch, route_pkt);
		a.AckCredits(src_ch, credits);
		out.Pick(*route_pkt);
		if (!routed)
			return false;
	}

	dev.sent = true;
	return true;
}

void HalUpp::GuiFileSrc_Visit(NativeGuiFileSrc& dev, AtomBase& a, Visitor& vis) {
	// Visit for serialization/inspection
}

void HalUpp::GuiFileSrc_Update(NativeGuiFileSrc& dev, AtomBase& a, double dt) {
	// Update for time-based operations
}

bool HalUpp::GuiFileSrc_IsReady(NativeGuiFileSrc& dev, AtomBase& a, PacketIO& io) {
	// Ready to send if loaded and not yet sent
	return dev.loaded && !dev.sent;
}

bool HalUpp::GuiFileSrc_AttachContext(NativeGuiFileSrc& dev, AtomBase& a, AtomBase& other) {
	return true;
}

void HalUpp::GuiFileSrc_DetachContext(NativeGuiFileSrc& dev, AtomBase& a, AtomBase& other) {
	// Detach context
}

// Stubs for missing HalUpp devices
#if (defined flagHAL && defined flagAUDIO)
struct HalUpp::NativeAudioSinkDevice {};
bool HalUpp::AudioSinkDevice_Create(NativeAudioSinkDevice*& dev) { return false; }
void HalUpp::AudioSinkDevice_Destroy(NativeAudioSinkDevice*& dev) {}
bool HalUpp::AudioSinkDevice_Initialize(NativeAudioSinkDevice&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::AudioSinkDevice_PostInitialize(NativeAudioSinkDevice&, AtomBase&) { return false; }
bool HalUpp::AudioSinkDevice_Start(NativeAudioSinkDevice&, AtomBase&) { return false; }
void HalUpp::AudioSinkDevice_Stop(NativeAudioSinkDevice&, AtomBase&) {}
void HalUpp::AudioSinkDevice_Uninitialize(NativeAudioSinkDevice&, AtomBase&) {}
bool HalUpp::AudioSinkDevice_Send(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::AudioSinkDevice_Visit(NativeAudioSinkDevice&, AtomBase&, Visitor&) {}
bool HalUpp::AudioSinkDevice_Recv(NativeAudioSinkDevice&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::AudioSinkDevice_Finalize(NativeAudioSinkDevice&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::AudioSinkDevice_Update(NativeAudioSinkDevice&, AtomBase&, double) {}
bool HalUpp::AudioSinkDevice_IsReady(NativeAudioSinkDevice&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::AudioSinkDevice_AttachContext(NativeAudioSinkDevice&, AtomBase&, AtomBase&) { return false; }
void HalUpp::AudioSinkDevice_DetachContext(NativeAudioSinkDevice&, AtomBase&, AtomBase&) {}
#endif

#if (defined flagHAL && defined flagSCREEN)
struct HalUpp::NativeCenterScreenSinkDevice {};
bool HalUpp::CenterScreenSinkDevice_Create(NativeCenterScreenSinkDevice*& dev) { return false; }
void HalUpp::CenterScreenSinkDevice_Destroy(NativeCenterScreenSinkDevice*& dev) {}
bool HalUpp::CenterScreenSinkDevice_Initialize(NativeCenterScreenSinkDevice&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::CenterScreenSinkDevice_PostInitialize(NativeCenterScreenSinkDevice&, AtomBase&) { return false; }
bool HalUpp::CenterScreenSinkDevice_Start(NativeCenterScreenSinkDevice&, AtomBase&) { return false; }
void HalUpp::CenterScreenSinkDevice_Stop(NativeCenterScreenSinkDevice&, AtomBase&) {}
void HalUpp::CenterScreenSinkDevice_Uninitialize(NativeCenterScreenSinkDevice&, AtomBase&) {}
bool HalUpp::CenterScreenSinkDevice_Send(NativeCenterScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::CenterScreenSinkDevice_Visit(NativeCenterScreenSinkDevice&, AtomBase&, Visitor&) {}
bool HalUpp::CenterScreenSinkDevice_Recv(NativeCenterScreenSinkDevice&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::CenterScreenSinkDevice_Finalize(NativeCenterScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::CenterScreenSinkDevice_Update(NativeCenterScreenSinkDevice&, AtomBase&, double) {}
bool HalUpp::CenterScreenSinkDevice_IsReady(NativeCenterScreenSinkDevice&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::CenterScreenSinkDevice_AttachContext(NativeCenterScreenSinkDevice&, AtomBase&, AtomBase&) { return false; }
void HalUpp::CenterScreenSinkDevice_DetachContext(NativeCenterScreenSinkDevice&, AtomBase&, AtomBase&) {}
#endif

#if (defined flagHAL && defined flagFBO)
struct HalUpp::NativeCenterFboSinkDevice {};
bool HalUpp::CenterFboSinkDevice_Create(NativeCenterFboSinkDevice*& dev) { return false; }
void HalUpp::CenterFboSinkDevice_Destroy(NativeCenterFboSinkDevice*& dev) {}
bool HalUpp::CenterFboSinkDevice_Initialize(NativeCenterFboSinkDevice&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::CenterFboSinkDevice_PostInitialize(NativeCenterFboSinkDevice&, AtomBase&) { return false; }
bool HalUpp::CenterFboSinkDevice_Start(NativeCenterFboSinkDevice&, AtomBase&) { return false; }
void HalUpp::CenterFboSinkDevice_Stop(NativeCenterFboSinkDevice&, AtomBase&) {}
void HalUpp::CenterFboSinkDevice_Uninitialize(NativeCenterFboSinkDevice&, AtomBase&) {}
bool HalUpp::CenterFboSinkDevice_Send(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::CenterFboSinkDevice_Visit(NativeCenterFboSinkDevice&, AtomBase&, Visitor&) {}
bool HalUpp::CenterFboSinkDevice_Recv(NativeCenterFboSinkDevice&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::CenterFboSinkDevice_Finalize(NativeCenterFboSinkDevice&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::CenterFboSinkDevice_Update(NativeCenterFboSinkDevice&, AtomBase&, double) {}
bool HalUpp::CenterFboSinkDevice_IsReady(NativeCenterFboSinkDevice&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::CenterFboSinkDevice_AttachContext(NativeCenterFboSinkDevice&, AtomBase&, AtomBase&) { return false; }
void HalUpp::CenterFboSinkDevice_DetachContext(NativeCenterFboSinkDevice&, AtomBase&, AtomBase&) {}
#endif

#if (defined flagHAL && defined flagOGL)
struct HalUpp::NativeOglScreenSinkDevice {};
bool HalUpp::OglScreenSinkDevice_Create(NativeOglScreenSinkDevice*& dev) { return false; }
void HalUpp::OglScreenSinkDevice_Destroy(NativeOglScreenSinkDevice*& dev) {}
bool HalUpp::OglScreenSinkDevice_Initialize(NativeOglScreenSinkDevice&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::OglScreenSinkDevice_PostInitialize(NativeOglScreenSinkDevice&, AtomBase&) { return false; }
bool HalUpp::OglScreenSinkDevice_Start(NativeOglScreenSinkDevice&, AtomBase&) { return false; }
void HalUpp::OglScreenSinkDevice_Stop(NativeOglScreenSinkDevice&, AtomBase&) {}
void HalUpp::OglScreenSinkDevice_Uninitialize(NativeOglScreenSinkDevice&, AtomBase&) {}
bool HalUpp::OglScreenSinkDevice_Send(NativeOglScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::OglScreenSinkDevice_Visit(NativeOglScreenSinkDevice&, AtomBase&, Visitor&) {}
bool HalUpp::OglScreenSinkDevice_Recv(NativeOglScreenSinkDevice&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::OglScreenSinkDevice_Finalize(NativeOglScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::OglScreenSinkDevice_Update(NativeOglScreenSinkDevice&, AtomBase&, double) {}
bool HalUpp::OglScreenSinkDevice_IsReady(NativeOglScreenSinkDevice&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::OglScreenSinkDevice_AttachContext(NativeOglScreenSinkDevice&, AtomBase&, AtomBase&) { return false; }
void HalUpp::OglScreenSinkDevice_DetachContext(NativeOglScreenSinkDevice&, AtomBase&, AtomBase&) {}
#endif

#if (defined flagHAL && defined flagDX12)
struct HalUpp::NativeD12ScreenSinkDevice {};
bool HalUpp::D12ScreenSinkDevice_Create(NativeD12ScreenSinkDevice*& dev) { return false; }
void HalUpp::D12ScreenSinkDevice_Destroy(NativeD12ScreenSinkDevice*& dev) {}
bool HalUpp::D12ScreenSinkDevice_Initialize(NativeD12ScreenSinkDevice&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::D12ScreenSinkDevice_PostInitialize(NativeD12ScreenSinkDevice&, AtomBase&) { return false; }
bool HalUpp::D12ScreenSinkDevice_Start(NativeD12ScreenSinkDevice&, AtomBase&) { return false; }
void HalUpp::D12ScreenSinkDevice_Stop(NativeD12ScreenSinkDevice&, AtomBase&) {}
void HalUpp::D12ScreenSinkDevice_Uninitialize(NativeD12ScreenSinkDevice&, AtomBase&) {}
bool HalUpp::D12ScreenSinkDevice_Send(NativeD12ScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::D12ScreenSinkDevice_Visit(NativeD12ScreenSinkDevice&, AtomBase&, Visitor&) {}
bool HalUpp::D12ScreenSinkDevice_Recv(NativeD12ScreenSinkDevice&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::D12ScreenSinkDevice_Finalize(NativeD12ScreenSinkDevice&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::D12ScreenSinkDevice_Update(NativeD12ScreenSinkDevice&, AtomBase&, double) {}
bool HalUpp::D12ScreenSinkDevice_IsReady(NativeD12ScreenSinkDevice&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::D12ScreenSinkDevice_AttachContext(NativeD12ScreenSinkDevice&, AtomBase&, AtomBase&) { return false; }
void HalUpp::D12ScreenSinkDevice_DetachContext(NativeD12ScreenSinkDevice&, AtomBase&, AtomBase&) {}
#endif

#if defined flagHAL
struct HalUpp::NativeContextBase {};
bool HalUpp::ContextBase_Create(NativeContextBase*& dev) { return false; }
void HalUpp::ContextBase_Destroy(NativeContextBase*& dev) {}
bool HalUpp::ContextBase_Initialize(NativeContextBase&, AtomBase&, const WorldState&) { return false; }
bool HalUpp::ContextBase_PostInitialize(NativeContextBase&, AtomBase&) { return false; }
bool HalUpp::ContextBase_Start(NativeContextBase&, AtomBase&) { return false; }
void HalUpp::ContextBase_Stop(NativeContextBase&, AtomBase&) {}
void HalUpp::ContextBase_Uninitialize(NativeContextBase&, AtomBase&) {}
bool HalUpp::ContextBase_Send(NativeContextBase&, AtomBase&, RealtimeSourceConfig&, PacketValue&, int) { return false; }
void HalUpp::ContextBase_Visit(NativeContextBase&, AtomBase&, Visitor&) {}
bool HalUpp::ContextBase_Recv(NativeContextBase&, AtomBase&, int, const Packet&) { return false; }
void HalUpp::ContextBase_Finalize(NativeContextBase&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::ContextBase_Update(NativeContextBase&, AtomBase&, double) {}
bool HalUpp::ContextBase_IsReady(NativeContextBase&, AtomBase&, PacketIO&) { return false; }
bool HalUpp::ContextBase_AttachContext(NativeContextBase&, AtomBase&, AtomBase&) { return false; }
void HalUpp::ContextBase_DetachContext(NativeContextBase&, AtomBase&, AtomBase&) {}
#endif

struct HalUpp::NativeUppOglDevice {};
bool HalUpp::NativeUppOglDevice_Create(NativeUppOglDevice*& dev) { return false; }
void HalUpp::NativeUppOglDevice_Destroy(NativeUppOglDevice*& dev) {}
bool HalUpp::NativeUppOglDevice_Initialize(NativeUppOglDevice& dev, AtomBase& a, const WorldState& ws) { return false; }
bool HalUpp::NativeUppOglDevice_PostInitialize(NativeUppOglDevice& dev, AtomBase& a) { return false; }
bool HalUpp::NativeUppOglDevice_Start(NativeUppOglDevice&, AtomBase&) { return false; }
void HalUpp::NativeUppOglDevice_Stop(NativeUppOglDevice&, AtomBase& a) {}
void HalUpp::NativeUppOglDevice_Uninitialize(NativeUppOglDevice& dev, AtomBase& a) {}
bool HalUpp::NativeUppOglDevice_Send(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) { return false; }
bool HalUpp::NativeUppOglDevice_Recv(NativeUppOglDevice&, AtomBase&, int sink_ch, const Packet& in) { return false; }
void HalUpp::NativeUppOglDevice_Finalize(NativeUppOglDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg) {}
void HalUpp::NativeUppOglDevice_Update(NativeUppOglDevice&, AtomBase&, double dt) {}
bool HalUpp::NativeUppOglDevice_IsReady(NativeUppOglDevice&, AtomBase&, PacketIO& io) { return false; }
void HalUpp::NativeUppOglDevice_Visit(NativeUppOglDevice&, AtomBase&, Vis& v) {}
bool HalUpp::NativeUppOglDevice_AttachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a) { return false; }
void HalUpp::NativeUppOglDevice_DetachContext(NativeUppOglDevice&, AtomBase&, AtomBase& a) {}

struct HalUpp::NativeUppEventsBase {};
bool HalUpp::UppEventsBase_Create(NativeUppEventsBase*& dev) { return false; }
void HalUpp::UppEventsBase_Destroy(NativeUppEventsBase*& dev) {}
bool HalUpp::UppEventsBase_Initialize(NativeUppEventsBase& dev, AtomBase& a, const WorldState&) { return false; }
bool HalUpp::UppEventsBase_PostInitialize(NativeUppEventsBase& dev, AtomBase& a) { return false; }
bool HalUpp::UppEventsBase_Start(NativeUppEventsBase&, AtomBase&) { return false; }
void HalUpp::UppEventsBase_Stop(NativeUppEventsBase&, AtomBase& a) {}
void HalUpp::UppEventsBase_Uninitialize(NativeUppEventsBase& dev, AtomBase& a) {}
bool HalUpp::UppEventsBase_Send(NativeUppEventsBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) { return false; }
bool HalUpp::UppEventsBase_Recv(NativeUppEventsBase&, AtomBase&, int sink_ch, const Packet& in) { return false; }
void HalUpp::UppEventsBase_Finalize(NativeUppEventsBase&, AtomBase&, RealtimeSourceConfig&) {}
void HalUpp::UppEventsBase_Update(NativeUppEventsBase&, AtomBase&, double dt) {}
bool HalUpp::UppEventsBase_IsReady(NativeUppEventsBase&, AtomBase&, PacketIO& io) { return false; }
void HalUpp::UppEventsBase_Visit(NativeUppEventsBase&, AtomBase&, Vis& v) {}
bool HalUpp::UppEventsBase_AttachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a) { return false; }
void HalUpp::UppEventsBase_DetachContext(NativeUppEventsBase&, AtomBase&, AtomBase& a) {}

END_UPP_NAMESPACE

#endif // flagHAL && flagGUI
