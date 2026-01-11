#include "Hal.h"

#if defined flagHAL && defined flagGUI

#include <Form/Form.hpp>

NAMESPACE_UPP


// Mid-class between Atom and FormWindow
// Provides room for future Esc script VM, event I/O, and threading
struct HalUpp::NativeUppGuiSinkBase {
	// Form display
	FormWindow form_win;

	// Event loop integration
	Ctrl::EventLoopContext event_ctx;
	bool event_ctx_active = false;

	// Future expansion points (not implemented yet):
	// - Esc::VM script_vm;              // Esc script virtual machine
	// - Vector<Thread> script_threads;  // Scripting thread pool
	// - PacketBuffer event_in;          // Incoming event packets
	// - PacketBuffer event_out;         // Outgoing event packets

	bool initialized = false;

	~NativeUppGuiSinkBase() {
		if (event_ctx_active) {
			Ctrl::EventLoopEnd(event_ctx);
			event_ctx_active = false;
		}
		form_win.Close();
	}
};

// Device interface functions
bool HalUpp::UppGuiSinkBase_Create(NativeUppGuiSinkBase*& dev) {
	dev = new NativeUppGuiSinkBase;
	return true;
}

void HalUpp::UppGuiSinkBase_Destroy(NativeUppGuiSinkBase*& dev) {
	if (dev) {
		delete dev;
		dev = nullptr;
	}
}

bool HalUpp::UppGuiSinkBase_Initialize(NativeUppGuiSinkBase& dev, AtomBase& a, const Script::WorldState& ws) {
	// Setup window from configuration
	bool sizeable = a.Get(".sizeable", true);
	bool close_machine = a.Get(".close_machine", true);

	if (sizeable)
		dev.form_win.Sizeable();

	if (close_machine)
		dev.form_win.WhenClose = [&a]() { a.GetMachine().SetNotRunning(); };

	// Window will be sized by form layout
	dev.form_win.Title("GUI Event Viewer");

	dev.initialized = true;
	return true;
}

bool HalUpp::UppGuiSinkBase_PostInitialize(NativeUppGuiSinkBase& dev, AtomBase& a) {
	return true;
}

bool HalUpp::UppGuiSinkBase_Start(NativeUppGuiSinkBase& dev, AtomBase& a) {
	return true;
}

void HalUpp::UppGuiSinkBase_Stop(NativeUppGuiSinkBase& dev, AtomBase& a) {
	a.ClearDependencies();
}

void HalUpp::UppGuiSinkBase_Uninitialize(NativeUppGuiSinkBase& dev, AtomBase& a) {
	if (dev.event_ctx_active) {
		Ctrl::EventLoopEnd(dev.event_ctx);
		dev.event_ctx_active = false;
	}
	dev.form_win.Close();
	dev.initialized = false;

	a.RemoveAtomFromUpdateList();
}

bool HalUpp::UppGuiSinkBase_Recv(NativeUppGuiSinkBase& dev, AtomBase& a, int sink_ch, const Packet& in) {
	if (!dev.initialized)
		return false;

	auto& fmt = in->GetFormat();
	if (!fmt.IsGui()) {
		LOG("UppGuiSinkDevice: expected GUI format, got " << fmt.ToString());
		return false;
	}

	// Extract GUI form data from packet
	// For now, we expect the packet data to be the form XML string
	int data_size = in->GetDataSize();
	if (data_size <= 0) {
		LOG("UppGuiSinkDevice: empty packet data");
		return false;
	}

	const byte* data_ptr = (const byte*)in->Data();
	String form_xml((const char*)data_ptr, data_size);

	// Load form from XML
	LOG("UppGuiSinkDevice: Loading form from XML (" << form_xml.GetCount() << " bytes)");
	if (!dev.form_win.LoadString(form_xml, false)) {
		LOG("UppGuiSinkDevice: failed to load form from XML");
		return false;
	}

	// Select default layout
	String layout_name = "Default";
	if (!dev.form_win.Layout(layout_name)) {
		LOG("UppGuiSinkDevice: layout '" << layout_name << "' not found");
		return false;
	}

	// Open window if not visible
	if (!dev.form_win.IsOpen()) {
		dev.form_win.Open();
		LOG("UppGuiSinkDevice: window opened");
	}

	return true;
}

void HalUpp::UppGuiSinkBase_Finalize(NativeUppGuiSinkBase& dev, AtomBase& a, RealtimeSourceConfig& cfg) {
	if (!dev.initialized || !dev.form_win.IsOpen())
		return;

	// Process one event loop iteration
	if (!dev.event_ctx_active) {
		dev.event_ctx = Ctrl::EventLoopBegin(NULL);
		dev.event_ctx_active = true;
	}

	Ctrl::EventLoopIteration(dev.event_ctx);
}

bool HalUpp::UppGuiSinkBase_Send(NativeUppGuiSinkBase& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
	// Send receipt acknowledging GUI display
	auto& fmt = out.SetFormat();
	fmt.SetReceipt(DevCls::CENTER);

	// TODO: Set receipt data (timestamp, status, etc.)

	return true;
}

void HalUpp::UppGuiSinkBase_Visit(NativeUppGuiSinkBase& dev, AtomBase& a, Visitor& vis) {
	// Visit for serialization/inspection
}

void HalUpp::UppGuiSinkBase_Update(NativeUppGuiSinkBase& dev, AtomBase& a, double dt) {
	// Update for time-based operations
}

bool HalUpp::UppGuiSinkBase_IsReady(NativeUppGuiSinkBase& dev, AtomBase& a, PacketIO& io) {
	// Always ready to receive GUI data
	return false; // We don't produce data, only receive
}

bool HalUpp::UppGuiSinkBase_AttachContext(NativeUppGuiSinkBase& dev, AtomBase& a, AtomBase& other) {
	return true;
}

void HalUpp::UppGuiSinkBase_DetachContext(NativeUppGuiSinkBase& dev, AtomBase& a, AtomBase& other) {
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

bool HalUpp::GuiFileSrc_Initialize(NativeGuiFileSrc& dev, AtomBase& a, const Script::WorldState& ws) {
	// Get file path from configuration
	dev.form_file_path = a.Get(".file", String());
	if (dev.form_file_path.IsEmpty()) {
		LOG("GuiFileSrc: no file path specified");
		return false;
	}

	// Load the .form file
	if (!LoadFile(dev.form_file_path, dev.form_xml_data)) {
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
	auto& fmt = in->GetFormat();
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
	auto& fmt = out.SetFormat();
	fmt.SetGui(DevCls::CENTER);

	// Set packet data to the form XML
	int data_size = dev.form_xml_data.GetCount();
	byte* data_ptr = (byte*)out.AllocData(data_size);
	memcpy(data_ptr, dev.form_xml_data.Begin(), data_size);

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
