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
	// Send receipt acknowledging GUI display
	ValueFormat fmt;
	fmt.SetReceipt(DevCls::CENTER);
	out.SetFormat(fmt);

	// TODO: Set receipt data (timestamp, status, etc.)

	return true;
}

void HalUpp::GuiSinkBase_Visit(NativeGuiSinkBase& dev, AtomBase& a, Visitor& vis) {
	// Visit for serialization/inspection
}

void HalUpp::GuiSinkBase_Update(NativeGuiSinkBase& dev, AtomBase& a, double dt) {
	// Update for time-based operations
}

bool HalUpp::GuiSinkBase_IsReady(NativeGuiSinkBase& dev, AtomBase& a, PacketIO& io) {
	// Always ready to receive GUI data
	return false; // We don't produce data, only receive
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
		// Reset sent flag to allow re-sending
		dev.sent = false;
		LOG("GuiFileSrc: received order, will resend form data");
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


END_UPP_NAMESPACE

#endif // flagHAL && flagGUI
