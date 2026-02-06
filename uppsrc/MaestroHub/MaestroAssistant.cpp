#include "MaestroHub.h"

NAMESPACE_UPP

MaestroAssistant::MaestroAssistant() {
	CtrlLayout(*this);
	
	context_stack.AddColumn("Layer");
	context_stack.AddColumn("Context");
	
	vsplit.Vert(chat, context_stack);
	vsplit.SetPos(7500); 
}

void MaestroAssistant::Toggle() {
	// Handled by MaestroHub via button action
}

void MaestroAssistant::UpdateContext(const String& track, const String& phase, const String& task) {
	context_stack.Clear();
	if(!track.IsEmpty()) context_stack.Add("Track", track);
	if(!phase.IsEmpty()) context_stack.Add("Phase", phase);
	if(!task.IsEmpty())  context_stack.Add("Task", task);
}

END_UPP_NAMESPACE
