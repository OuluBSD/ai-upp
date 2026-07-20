#include <Core/Core.h>
#include <Vfs/ProgDB/ProgDB.h>

using namespace UPP;

const int ADE_PORT = 12345;

bool SendClientRequest(const ValueArray& args, String& out_stdout, String& out_stderr) {
	TcpSocket client;
	if(!client.Connect("127.0.0.1", ADE_PORT)) {
		out_stderr = "ade: cannot connect to server on port 12345. Is the server running? Start it with: ade server --root <path>";
		return false;
	}
	
	ValueMap req;
	req.Set("args", args);
	String req_str = AsJSON(req);
	client.PutAll(req_str + "\n");
	
	String resp_str = client.GetLine();
	Value resp_val = ParseJSON(resp_str);
	if(resp_val.IsError() || !resp_val.Is<ValueMap>()) {
		out_stderr = "ade: invalid response from server";
		return false;
	}
	
	ValueMap resp = resp_val;
	if(resp["status"] == "ok") {
		out_stdout = resp["output"];
		return true;
	} else {
		out_stderr = resp["message"];
		return false;
	}
}

void ProcessServerRequest(TcpSocket& client, ProgDatabase& db) {
	String req_str = client.GetLine();
	Value req_val = ParseJSON(req_str);
	if(req_val.IsError() || !req_val.Is<ValueMap>()) {
		ValueMap resp;
		resp.Set("status", "error");
		resp.Set("message", "invalid JSON request");
		client.PutAll(AsJSON(resp) + "\n");
		return;
	}

	ValueMap req = req_val;
	ValueArray args = req["args"];
	if(args.IsEmpty()) {
		ValueMap resp;
		resp.Set("status", "error");
		resp.Set("message", "empty arguments");
		client.PutAll(AsJSON(resp) + "\n");
		return;
	}

	String cmd = args[0];
	ValueMap resp;
	resp.Set("status", "ok");
	
	if(cmd == "status") {
		String out;
		out << "server: running\n";
		out << "root: " << db.GetRootDirectory() << "\n";
		out << "nodes: " << db.GetNodeCount() << "\n";
		resp.Set("output", out);
	}
	else if(cmd == "list") {
		String category = args.GetCount() > 1 ? args[1] : "";
		Vector<ProgNodeId> list;
		if(category == "units") {
			list = db.QueryByKind(PROG_NODE_UNIT);
		} else if(category == "symbols") {
			list = db.QueryByKind(PROG_NODE_NAMESPACE);
			list.Append(db.QueryByKind(PROG_NODE_CLASS));
			list.Append(db.QueryByKind(PROG_NODE_STRUCT));
			list.Append(db.QueryByKind(PROG_NODE_ENUM));
			list.Append(db.QueryByKind(PROG_NODE_VARIABLE));
		} else if(category == "functions") {
			list = db.QueryByKind(PROG_NODE_FUNCTION);
		} else if(category == "comments") {
			list = db.QueryByKind(PROG_NODE_COMMENT);
		} else if(category == "tags") {
			list = db.QueryByKind(PROG_NODE_TAG);
		} else if(category == "target-api-map") {
			list = db.QueryByKind(PROG_NODE_TARGET_BINDING);
		} else {
			for(int k = PROG_NODE_UNIT; k <= PROG_NODE_TARGET_BINDING; k++) {
				Vector<ProgNodeId> sub = db.QueryByKind((ProgNodeKind)k);
				list.Append(sub);
			}
		}
		
		String out;
		for(const auto& id : list) {
			out << id.value << "\n";
		}
		resp.Set("output", out);
	}
	else if(cmd == "get") {
		if(args.GetCount() < 2) {
			resp.Set("status", "error");
			resp.Set("message", "missing node ID");
		} else {
			String id_val = args[1];
			const ProgNodeRecord* rec = db.FindNode(ProgNodeId(id_val));
			if(!rec) {
				resp.Set("status", "error");
				resp.Set("message", "node not found: " + id_val);
			} else {
				resp.Set("output", rec->ToPLF());
			}
		}
	}
	else if(cmd == "put") {
		if(args.GetCount() < 3) {
			resp.Set("status", "error");
			resp.Set("message", "missing node ID or content");
		} else {
			String id_val = args[1];
			String content = args[2];
			ProgNodeRecord rec;
			if(!rec.LoadPLF(content)) {
				resp.Set("status", "error");
				resp.Set("message", "failed to parse node content in PLF format");
			} else {
				rec.id.value = id_val; // make sure ID matches command parameter
				db.AddNode(rec);
				db.Save();
				resp.Set("output", "node added/updated: " + id_val + "\n");
			}
		}
	}
	else if(cmd == "rename") {
		if(args.GetCount() < 3) {
			resp.Set("status", "error");
			resp.Set("message", "missing node ID or new name");
		} else {
			String id_val = args[1];
			String new_name = args[2];
			if(db.RenameNode(ProgNodeId(id_val), new_name)) {
				resp.Set("output", "node renamed to name: " + new_name + "\n");
			} else {
				resp.Set("status", "error");
				resp.Set("message", "failed to rename node: " + id_val);
			}
		}
	}
	else if(cmd == "rename-id") {
		if(args.GetCount() < 3) {
			resp.Set("status", "error");
			resp.Set("message", "missing old ID or new ID");
		} else {
			String old_id = args[1];
			String new_id = args[2];
			if(db.RenameNodeId(ProgNodeId(old_id), ProgNodeId(new_id))) {
				resp.Set("output", "node ID renamed from " + old_id + " to " + new_id + "\n");
			} else {
				resp.Set("status", "error");
				resp.Set("message", "failed to rename node ID: " + old_id);
			}
		}
	}
	else if(cmd == "delete") {
		if(args.GetCount() < 2) {
			resp.Set("status", "error");
			resp.Set("message", "missing node ID");
		} else {
			String id_val = args[1];
			bool simulate = false;
			if(args.GetCount() > 2 && args[2] == "--simulate") {
				simulate = true;
			}
			
			if(simulate) {
				Vector<ProgNodeId> rels = db.QueryRelations("belongs_to", ProgNodeId(id_val));
				Vector<ProgNodeId> children = db.QueryChildren(ProgNodeId(id_val));
				
				String out;
				out << "Simulation: delete " << id_val << "\n";
				out << "Affected children:\n";
				for(const auto& child : children) out << "  " << child.value << "\n";
				out << "Affected relations:\n";
				for(const auto& rel : rels) out << "  " << rel.value << "\n";
				resp.Set("output", out);
			} else {
				if(db.RemoveNode(ProgNodeId(id_val))) {
					resp.Set("output", "node deleted: " + id_val + "\n");
				} else {
					resp.Set("status", "error");
					resp.Set("message", "failed to delete node: " + id_val);
				}
			}
		}
	}
	else if(cmd == "search") {
		if(args.GetCount() < 2) {
			resp.Set("status", "error");
			resp.Set("message", "missing search query");
		} else {
			String tag = args[1];
			Vector<ProgNodeId> res = db.QueryByTag(tag);
			String out;
			for(const auto& id : res) out << id.value << "\n";
			resp.Set("output", out);
		}
	}
	else if(cmd == "comment") {
		if(args.GetCount() < 4 || args[1] != "add") {
			resp.Set("status", "error");
			resp.Set("message", "usage: comment add <node_id> <comment_text>");
		} else {
			String node_id = args[2];
			String comment_text = args[3];
			
			ProgNodeRecord comment_rec;
			comment_rec.id.value = "comment." + Uuid::Create().ToString();
			comment_rec.kind = PROG_NODE_COMMENT;
			comment_rec.name = "Comment on " + node_id;
			comment_rec.parent.value = node_id;
			
			ProgRelation& rel = comment_rec.relations.Add();
			rel.type = "comment_on";
			rel.target.value = node_id;
			
			comment_rec.data.Set("text", comment_text);
			
			db.AddNode(comment_rec);
			db.Save();
			resp.Set("output", "comment added: " + comment_rec.id.value + "\n");
		}
	}
	else {
		resp.Set("status", "error");
		resp.Set("message", "unknown command: " + cmd);
	}
	
	client.PutAll(AsJSON(resp) + "\n");
}

void RunServer(const String& db_path) {
	ProgDatabase db;
	if(!db.Open(db_path)) {
		Cerr() << "ade server: failed to open database at " << db_path << "\n";
		Exit(1);
	}
	
	TcpSocket server;
	if(!server.Listen(ADE_PORT, 5)) {
		Cerr() << "ade server: failed to listen on port " << ADE_PORT << "\n";
		Exit(1);
	}
	
	Cout() << "ade server: listening on port " << ADE_PORT << " with root " << db_path << "\n";
	
	while(true) {
		TcpSocket client;
		if(client.Accept(server)) {
			ProcessServerRequest(client, db);
		}
	}
}

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.IsEmpty() || args[0] == "help" || args[0] == "--help") {
		Cout() << "ade: agentic development environment\n";
		Cout() << "commands:\n";
		Cout() << "  server --root <path>           Start persistent backend server\n";
		Cout() << "  status                         Show server database status\n";
		Cout() << "  list [<category>]              List node IDs\n";
		Cout() << "  get <node_id>                  Get node details in PLF format\n";
		Cout() << "  put <node_id> <file_path>      Put node content from a file\n";
		Cout() << "  rename <node_id> <new_name>    Rename node name\n";
		Cout() << "  rename-id <old_id> <new_id>    Rename node ID (referential integrity!)\n";
		Cout() << "  delete <node_id> [--simulate]  Delete a node\n";
		Cout() << "  search <tag>                   Search nodes by tag\n";
		Cout() << "  comment add <node_id> <text>   Add a comment to a node\n";
		SetExitCode(0);
		return;
	}
	
	if(args[0] == "server") {
		String db_path;
		for(int i = 1; i < args.GetCount() - 1; i++) {
			if(args[i] == "--root") {
				db_path = args[i+1];
				break;
			}
		}
		if(db_path.IsEmpty()) {
			Cerr() << "ade server: missing --root <path>\n";
			SetExitCode(1);
			return;
		}
		RunServer(db_path);
		return;
	}
	
	ValueArray client_args;
	for(int i = 0; i < args.GetCount(); i++) {
		client_args.Add(args[i]);
	}
	
	if(args[0] == "put") {
		if(args.GetCount() < 3) {
			Cerr() << "ade client put: usage: ade put <node_id> <file_path>\n";
			SetExitCode(1);
			return;
		}
		String file_path = args[2];
		if(!FileExists(file_path)) {
			Cerr() << "ade client put: file not found: " << file_path << "\n";
			SetExitCode(1);
			return;
		}
		String content = LoadFile(file_path);
		client_args.Set(2, content);
	}
	
	String out_stdout, out_stderr;
	if(SendClientRequest(client_args, out_stdout, out_stderr)) {
		Cout() << out_stdout;
		SetExitCode(0);
	} else {
		Cerr() << out_stderr << "\n";
		SetExitCode(1);
	}
}
