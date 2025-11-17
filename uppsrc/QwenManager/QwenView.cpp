#include "QwenManager.h"
// NOTE: Intentionally not including QwenConnection.h to avoid circular dependencies
// QwenConnection functionality would be integrated through a different architectural approach

NAMESPACE_UPP

QwenProjectView::Entry::Entry() {

}

void QwenProjectView::Entry::SetDocText(bool view_only) {
	Add(doc.SizePos());
	doc.SetEditable(!view_only);
}

QwenProjectView::QwenProjectView() {
	// Set up status label
	status_label.SetFrame(BlackFrame());
	status_label.SetLabel("Disconnected");

	// Set up conversation history
	conversation_history.SetReadOnly();

	// Set up input area
	user_input <<= THISBACK(OnSend); // When Enter is pressed
	send_btn.SetLabel("Send");
	send_btn <<= THISBACK(OnSend);

	// Create input area
	Ctrl input_ctrl;
	input_ctrl.Add(user_input.HSizePos(2, 102).VSizePos(0, 30));  // Leave space for button
	input_ctrl.Add(send_btn.RightPos(2, 100).VSizePos(0, 30));

	// Set up a vertical splitter for the right side - conversation area on top, terminal below
	Splitter main_vsplit;
	main_vsplit.Vert();

	// Create a container for the conversation area components
	Ctrl conv_container;
	conv_container.Add(status_label.TopPos(0, 25).HSizePos());
	conv_container.Add(conversation_history.VSizePos(25, 30).HSizePos());  // Leave space for input
	conv_container.Add(input_ctrl.BottomPos(0, 30).HSizePos());

	// Split right side into conversation area and terminal
	main_vsplit << conv_container << term;

	// Set up the main horizontal splitter
	Splitter main_split;
	main_split.Horz() << page << main_vsplit;
	Add(main_split.SizePos());

}


void QwenProjectView::Data() {
	if (!prj) return;

	int c = prj->session_ids.GetCount(); // Get the count of entries in qwen session

	// Add new entries if needed
	for(int i = page.GetCount(); i < c; i++) {
		Entry& e = entries.Add();
		page.Add(e, "Session " + IntStr(i+1)); // Set a default title
		e.SetDocText();
	}

	// Update existing entries
	for(int i = 0; i < min(c, page.GetCount()); i++) {
		page.GetItem(i).Text(prj->session_ids[i]); // Update title with session ID
	}

	// Update connection status
	UpdateConnectionStatus();
}

void QwenProjectView::RefreshConversation() {
	// In a real implementation, we'd get conversation history from the QwenConnection
	// For now, we'll just display placeholder content

	String content;
	content << "No active session.\r\n";

	if (prj && prj->srv) {
		content << "Connected to: " << prj->srv->GetAddress() << "\r\n";
		content << "Status: " << prj->srv->GetStatusString() << "\r\n";
	}

	conversation_history.Clear();
	conversation_history <<= content;
}

void QwenProjectView::OnSend() {
	if (!prj) return;

	String user_text = ~user_input;  // Get value from LineEdit

	if (user_text.IsEmpty()) return;

	// Add user message to conversation
	String current_content = ~conversation_history; // Get current text content
	current_content << "You: " << user_text << "\r\n";

	// For now, this is a UI placeholder without actual connection functionality
	// In a complete implementation, this would send the text to the Qwen connection
	// LOG("Sending to Qwen: " << user_text);

	// Clear input field
	user_input.Clear();

	// Update conversation display
	conversation_history <<= current_content; // Set the new content

	// Update connection status
	UpdateConnectionStatus();
}

void QwenProjectView::UpdateConnectionStatus() {
	String status = "Status: ";

	if (prj && prj->srv) {
		status += prj->srv->GetStatusString();
		status += " | Connected to: " + prj->srv->GetAddress();
	} else {
		status += "Not connected";
	}

	// Update UI status display
	status_label.SetLabel(status);

	// Color-code the status
	Color status_color = (prj && prj->srv && prj->srv->is_connected) ? Green() : Red();
	status_label.SetInk(status_color);
}

END_UPP_NAMESPACE
