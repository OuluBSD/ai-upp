#include "LLDB.h"


LLDBDebuggerApp::LLDBDebuggerApp() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << lsplit << csplit << rsplit;
	hsplit.SetPos(1500,0);
	hsplit.SetPos(10000-1500,0);
	
	lsplit.Vert() << files;
	
	csplit.Vert() << ftabs << btabs;
	csplit.SetPos(10000-1800);
	
	btabs.Add(console.SizePos(), "Console");
	btabs.Add(normal.SizePos(), "Normal");
	btabs.Add(error.SizePos(), "Error");
	CtrlLayout(console);
	
	rsplit.Vert() << threads << stack << loctabs << breaktabs << dbg_stream;
	CtrlLayout(threads);
	
	stack.AddColumn("Function");
	stack.AddColumn("File");
	stack.AddColumn("Line");
	stack.AddIndex("IDX");
	stack.ColumnWidths("6 2 1");
	
	loctabs.Add(locals.SizePos(), "Locals");
	locals.AddColumn("Name");
	locals.AddColumn("Type");
	locals.AddColumn("Value");
	locals.AddIndex("IDX");
	locals.ColumnWidths("6 2 1");
	
	loctabs.Add(regs.SizePos(), "Registers");
	regs.AddColumn("Name");
	regs.AddColumn("Type");
	regs.AddColumn("Value");
	regs.AddIndex("IDX");
	regs.ColumnWidths("6 2 1");
	
	breaktabs.Add(breaks.SizePos(), "Breaks");
	breaks.AddColumn("File");
	breaks.AddColumn("Line");
	breaks.AddIndex("IDX");
	breaks.ColumnWidths("6 2");
	
	breaktabs.Add(watchs.SizePos(), "Waches");
	watchs.AddColumn("Variable");
	watchs.AddColumn("Value");
	watchs.AddIndex("IDX");
	watchs.ColumnWidths("6 2");
	
}

void LLDBDebuggerApp::Data() {
	
	// Locals
	{
		if (process.has_value() && process_is_stopped(*process)) {
            lldb::SBThread viewed_thread = process->GetThreadAtIndex(ui.viewed_thread_index);
            lldb::SBFrame frame = viewed_thread.GetFrameAtIndex(ui.viewed_frame_index);
            lldb::SBValueList locals = frame.GetVariables(true, true, true, true);

            // TODO: select entire row like in stack trace
            for (int i = 0; i < locals.GetSize(); i++) {
                DataLocalRecursive(locals.GetValueAtIndex(i));
            }
        }
	}
}

void LLDBDebuggerApp::DataLocalRecursive(lldb::SBValue local) {
	String local_type = local.GetDisplayTypeName();
    String local_name = local.GetName();
    String local_value = local.GetValue();

    if (!~local_type || !~local_name) {
        return;
    }

    String children_node_label = Format("%s##Children_%d", local_name, (int)local.GetID());

    if (local.MightHaveChildren()) {
        asdfadsf
        if (ImGui::TreeNode(~children_node_label)) {
            ImGui::NextColumn();
            ImGui::TextUnformatted(local_type);
            ImGui::NextColumn();
            ImGui::TextUnformatted("...");
            ImGui::NextColumn();

            // TODO: figure out best way to handle very long children list
            for (int i = 0; i < local.GetNumChildren(100); i++) {
                draw_local_recursive(local.GetChildAtIndex(i));
            }
            ImGui::TreePop();
        }
        else {
            ImGui::NextColumn();
            ImGui::TextUnformatted(local_type);
            ImGui::NextColumn();
            ImGui::TextUnformatted("...");
            ImGui::NextColumn();
        }
    }
    else {
        ImGui::TextUnformatted(local_name);
        ImGui::NextColumn();
        ImGui::TextUnformatted(local_type);
        ImGui::NextColumn();
        if (local_value.Is()) {
            ImGui::TextUnformatted(local_value);
        }
        else {
            ImGui::TextUnformatted("unknown");
        }
        ImGui::NextColumn();
    }
}
