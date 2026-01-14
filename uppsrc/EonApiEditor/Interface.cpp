#include "EonApiEditor.h"

NAMESPACE_UPP


InterfaceBuilder::Header& InterfaceBuilder::AddHeader(String name, String base, String role) {
	ASSERT(HasBase(base));
	Header& h = headers.Add();
	h.name = name;
	h.base = base;
	h.role = role;
	return h;
}

void InterfaceBuilder::Package(String s, String a) {
	cur = &packages.Add();
	cur->name = s;
	cur->abbr = a;
}

void InterfaceBuilder::SetColor(byte r, byte g, byte b) {
	cur->clr = UPP::Color(r,g,b);
}

void InterfaceBuilder::Dependency(String lib, String conditional, bool have_header) {
	struct Dependency& d = cur->deps.Add(lib);
	d.conditional = conditional;
	d.have_header = have_header;
}

void InterfaceBuilder::Library(String lib, String conditional) {
	cur->libs.Add(lib,conditional);
}

void InterfaceBuilder::HaveRecvFinalize() {
	cur->have_recv_finalize = true;
}

void InterfaceBuilder::HaveUpdate() {
	cur->have_update = true;
}

void InterfaceBuilder::HaveIsReady() {
	cur->have_is_ready = true;
}

void InterfaceBuilder::HaveNegotiateFormat() {
	cur->have_negotiate_format = true;
}

void InterfaceBuilder::HaveContextFunctions() {
	cur->have_context_fns = true;
}

void InterfaceBuilder::HaveSoundFunctions() {
	cur->have_sound_fns = true;
}

void InterfaceBuilder::HaveDebugFunctions() {
	cur->have_debug_fns = true;
}

void InterfaceBuilder::EnableIf(String conditional) {
	cur->enable_if.Add(conditional);
}

void InterfaceBuilder::EnableAlways() {
	cur->enable_always = true;
}

void InterfaceBuilder::Interface(String name, String conditional) {
	cur->ifaces.Add(name, conditional);
}

void InterfaceBuilder::Vendor(String name, String conditional) {
	cur->vendors.Add(name, conditional);
}





InterfaceBuilder::Header& InterfaceBuilder::Header::In(String vd) {
	ins.Add(vd, false);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::InOpt(String vd) {
	ins.Add(vd, true);
	return *this;
}


InterfaceBuilder::Header& InterfaceBuilder::Header::InOpt(int count, String vd) {
	for(int i = 0; i < count; i++)
		ins.Add(vd, true);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::Out(String vd) {
	outs.Add(vd, false);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::OutOpt(String vd) {
	outs.Add(vd, true);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::OutOpt(int count, String vd) {
	for(int i = 0; i < count; i++)
		outs.Add(vd, true);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::Action(String act) {
	ASSERT(action.IsEmpty());
	action = act;
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::Arg(String key, String value) {
	args.Add(key, value);
	return *this;
}

InterfaceBuilder::Header& InterfaceBuilder::Header::Link(String type, String role) {
	link_type = type;
	link_role = role;
	return *this;
}



void InterfaceBuilder::Generate(bool write_actually) {
	VectorMap<String, String> outputs;

	String prj_dir = root_path;
	String par_dir = AppendFileName(prj_dir, "uppsrc");
	String api_dir = AppendFileName(par_dir, "api");
	String eon_lib_dir = AppendFileName(par_dir, "Eon" DIR_SEPS "Lib");
	
	
	// Generate API packages
	for (const Pkg& pkg : packages) {
		String a = pkg.abbr;
		String n = pkg.name;
		String iname = "I" + n;  // For header guards only
		String dir = AppendFileName(api_dir, n);
		LOG("Package directory: " << dir);
		RealizeDirectory(dir);

		String upp_file = AppendFileName(dir, n + ".upp");
		LOG("\tProject file: " << upp_file);
		String h_file = AppendFileName(dir, n + ".h");
		LOG("\tHeader file: " << h_file);
		String iface_file = AppendFileName(dir, "IfaceFuncs.inl");
		LOG("\tIface file: " << iface_file);
		String impl_inl_file = AppendFileName(dir, "Impl.inl");
		LOG("\tImpl file: " << impl_inl_file);
		
		String cabbr = ToUpper(pkg.abbr);
		
		// .upp file
		{
			String s;
			int r = pkg.clr.GetR();
			int g = pkg.clr.GetG();
			int b = pkg.clr.GetB();
			
			s	<< "description \"\\377B" << IntStr(r) << "," << IntStr(g) << "," << IntStr(b) << "\";\n"
				<< "\n";
				
			for(int i = 0; i < pkg.deps.GetCount(); i++) {
				String k = pkg.deps.GetKey(i);
				const struct Dependency& dep = pkg.deps[i];
				
				if (dep.conditional.IsEmpty())
					s << "\tuses " << k << ";\n";
				else
					s << "\tuses(" << dep.conditional << ") " << k << ";\n";
			}
			
			s	<< "\n";
			
			for(int i = 0; i < pkg.libs.GetCount(); i++) {
				String k = pkg.libs.GetKey(i);
				String v = pkg.libs[i];
				if (v.IsEmpty())
					s << "\tlibrary \"" << k << "\";\n";
				else
					s << "\tlibrary(" << v << ") \"" << k << "\";\n";
			}
			
			s	<< "\n"
				<< "file\n"
				<< "\t" << n << ".h,\n"
				;
			
			for(int i = 0; i < pkg.vendors.GetCount(); i++) {
				String k = pkg.vendors.GetKey(i);
				s << "\t" << k << ".cpp,\n";
			}
			
			if (FileExists(impl_inl_file)) {
				s << "\tImpl.inl highlight cpp,\n";
			}
			
			s	<< "\tIfaceFuncs.inl highlight cpp;\n"
				<< "\n"
				<< "mainconfig\n"
				<< "        \"\" = \"\";\n";
			
			//LOG(s);
			outputs.Add(upp_file, s);
		}
		
		// Main header
		{
			String s;
			s	<< "// This file have been generated automatically.\n"
				<< "// DO NOT MODIFY THIS FILE!\n"
				<< "\n"
				<< "#ifndef _" << iname << "_" << iname << "_h_\n"
				<< "#define _" << iname << "_" << iname << "_h_\n"
				<< "\n"
				<< "#include <Eon/Eon.h>\n";

			for(int i = 0; i < pkg.deps.GetCount(); i++) {
				String k = pkg.deps.GetKey(i);
				const struct Dependency& dep = pkg.deps[i];
				if (dep.have_header) {
					String title = GetFileTitle(k);
					int j = title.ReverseFind("/");
					if (j >= 0)
						title = title.Mid(j+1);

					// Add flag guard if conditional dependency
					if (dep.conditional.GetCount())
						s << "#if " << GetMacroConditionals(dep.conditional) << "\n";

					s << "#include <" << k << "/" << title << ".h>\n";

					if (dep.conditional.GetCount())
						s << "#endif\n";
				}
			}
			
			s	<< "\n"
				<< "NAMESPACE_UPP\n"
				<< "\n"
				<< "#define " << cabbr << "_CLS_LIST(x) \\\n";
			
			for(int i = 0; i < pkg.ifaces.GetCount(); i++) {
				String k = pkg.ifaces.GetKey(i);
				s << "\t" << cabbr << "_CLS(" << k << ", x) \\\n";
			}
			
			s	<< "\n"
				<< "#define " << cabbr << "_VNDR_LIST \\\n";
			
			for(int i = 0; i < pkg.vendors.GetCount(); i++) {
				String k = pkg.vendors.GetKey(i);
				s << "\t" << cabbr << "_VNDR(" << pkg.abbr << k << ") \\\n";
			}
			
			s	<< "\n"
				<< "#define " << cabbr << "_CLS(x, v) struct v##x;\n"
				<< "#define " << cabbr << "_VNDR(x) " << cabbr << "_CLS_LIST(x)\n"
				<< "" << cabbr << "_VNDR_LIST\n"
				<< "#undef " << cabbr << "_VNDR\n"
				<< "#undef " << cabbr << "_CLS\n"
				<< "\n";
			
			ASSERT(!pkg.enable_always || pkg.enable_if.IsEmpty());
			
			for(int i = 0; i < pkg.vendors.GetCount(); i++) {
				String k = pkg.vendors.GetKey(i);
				String v = pkg.vendors[i];
				
				if (v.GetCount())
					s << "#if " << GetMacroConditionals(v) << "\n";
				
				
				s	<< "struct " << pkg.abbr << k << " {\n";
				
				for (int j = 0; j < pkg.ifaces.GetCount(); j++) {
					String k = pkg.ifaces.GetKey(j);
					String v = pkg.ifaces[j];
					
					if (v.GetCount())
						s << "\t#if " << GetMacroConditionals(v) << "\n";
					
					s << "\tstruct Native" << k << ";\n";
					
					if (v.GetCount())
						s << "\t#endif\n";
				}
				
				s	<< "\t\n"
					<< "\tstruct Thread {\n"
					<< "\t\t\n"
					<< "\t};\n"
					<< "\t\n"
					<< "\tstatic Thread& Local() {thread_local static Thread t; return t;}\n"
					<< "\t\n"
					<< "\t#include \"IfaceFuncs.inl\"\n"
					<< "\t\n"
					<< "};\n";
				
				if (v.GetCount())
					s << "#endif\n";
				
				
			}
			
			s	<< "\n";
			
			for(int i = 0; i < pkg.ifaces.GetCount(); i++) {
				String k = pkg.ifaces.GetKey(i);
				String v = pkg.ifaces[i];
				
				if (v.GetCount())
					s << "#if " << GetMacroConditionals(v) << "\n";
				
				s	<< "struct " << pkg.abbr << k << " : public Atom {\n"
					//<< "\t//RTTI_DECL1(" << pkg.abbr << k << ", Atom)\n"
					<< "\tusing Atom::Atom;\n"
					<< "\tvoid Visit(Vis& v) override {VIS_THIS(Atom);}\n"
					<< "\t\n"
					<< "\tvirtual ~" << pkg.abbr << k << "() {}\n"
					<< "};\n";
				
				if (v.GetCount())
					s << "#endif\n";
				
				s << "\n";
			}
			
			s	<< "\n";
			
			
			for(int i = 0; i < pkg.ifaces.GetCount(); i++) {
				String k = pkg.ifaces.GetKey(i);
				String v = pkg.ifaces[i];
				
				if (v.GetCount())
					s << "#if " << GetMacroConditionals(v) << "\n";
				
				s	<< "template <class " << a << "> struct " << n << k << "T : "<<a<<k<<" {\n"
				
					<< "\tusing CLASSNAME = "<<n<<k<<"T<"<<a<<">;\n"
					//<< "\t//RTTI_DECL1(CLASSNAME, "<<a<<k<<")\n"
					<< "\tusing "<<a<<k<<"::"<<a<<k<<";\n"
					<< "\tvoid Visit(Vis& v) override {\n"
					   "\t\tif (dev) "<<a<<"::"<<k<<"_Visit(*dev, *this, v);\n"
					   "\t\tVIS_THIS("<<a<<k<<");\n"
					   "\t}\n"
					
					<< "\ttypename "<<a<<"::Native"<<k<<"* dev = 0;\n"
					
					<< "\tbool Initialize(const WorldState& ws) override {\n"
				    << "\t\tif (!"<<a<<"::"<<k<<"_Create(dev))\n"
				    << "\t\t\treturn false;\n"
				    << "\t\tif (!"<<a<<"::"<<k<<"_Initialize(*dev, *this, ws))\n"
				    << "\t\t\treturn false;\n"
				    << "\t\treturn true;\n"
					<< "\t}\n"
					
					<< "\tbool PostInitialize() override {\n"
				    << "\t\tif (!"<<a<<"::"<<k<<"_PostInitialize(*dev, *this))\n"
				    << "\t\t\treturn false;\n"
				    << "\t\treturn true;\n"
					<< "\t}\n"
					
					<< "\tbool Start() override {\n"
				    << "\t\treturn "<<a<<"::"<<k<<"_Start(*dev, *this);\n"
					<< "\t}\n"
					
					<< "\tvoid Stop() override {\n"
				    << "\t\t"<<a<<"::"<<k<<"_Stop(*dev, *this);\n"
					<< "\t}\n"
					
					<< "\tvoid Uninitialize() override {\n"
				    << "\t\tASSERT(this->GetDependencyCount() == 0);\n"
				    << "\t\t"<<a<<"::"<<k<<"_Uninitialize(*dev, *this);\n"
				    << "\t\t"<<a<<"::"<<k<<"_Destroy(dev);\n"
					<< "\t}\n"
					
					<< "\tbool Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) override {\n"
				    << "\t\tif (!"<<a<<"::"<<k<<"_Send(*dev, *this, cfg, out, src_ch))\n"
				    << "\t\t\treturn false;\n"
				    << "\t\treturn true;\n"
					<< "\t}\n";
				
				if (pkg.have_recv_finalize)
					s	<< "\tbool Recv(int sink_ch, const Packet& in) override {\n"
					    << "\t\treturn "<<a<<"::"<<k<<"_Recv(*dev, *this, sink_ch, in);\n"
						<< "\t}\n"
					
						<< "\tvoid Finalize(RealtimeSourceConfig& cfg) override {\n"
					    << "\t\treturn "<<a<<"::"<<k<<"_Finalize(*dev, *this, cfg);\n"
						<< "\t}\n";
				
				if (pkg.have_update)
					s	<< "\tvoid Update(double dt) override {\n"
					    << "\t\treturn "<<a<<"::"<<k<<"_Update(*dev, *this, dt);\n"
						<< "\t}\n";
				
				if (pkg.have_is_ready)
					s	<< "\tbool IsReady(PacketIO& io) override {\n"
					    << "\t\treturn "<<a<<"::"<<k<<"_IsReady(*dev, *this, io);\n"
						<< "\t}\n";
				
				if (pkg.have_negotiate_format)
					s	<< "\tbool NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) override {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_NegotiateSinkFormat(*dev, *this, link, sink_ch, new_fmt);\n"
						<< "\t}\n";
				
				if (pkg.have_context_fns) {
					s	<< "\tbool AttachContext(AtomBase& a) override {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_AttachContext(*dev, *this, a);\n"
						<< "\t}\n";
						
					s	<< "\tvoid DetachContext(AtomBase& a) override {\n"
						<< "\t\t"<<a<<"::"<<k<<"_DetachContext(*dev, *this, a);\n"
						<< "\t}\n";
				}
				
				if (pkg.have_sound_fns) {
					s	<< "\tbool IsDebugSoundEnabled() const {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_IsDebugSoundEnabled(*dev, *this);\n"
						<< "\t}\n"
						<< "\tString GetDebugSoundOutput() const {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_GetDebugSoundOutput(*dev, *this);\n"
						<< "\t}\n"
						<< "\tint GetDebugSoundSeed() const {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_GetDebugSoundSeed(*dev, *this);\n"
						<< "\t}\n";
				}
				
				if (pkg.have_debug_fns) {
					s	<< "\tbool IsDebugPrintEnabled() const {\n"
						<< "\t\treturn "<<a<<"::"<<k<<"_IsDebugPrintEnabled(*dev, *this);\n"
						<< "\t}\n";
				}
				s	<< "};\n";
				
				if (v.GetCount())
					s << "#endif\n";
				
			}
			
			s << "\n";
			
			for(int i = 0; i < pkg.vendors.GetCount(); i++) {
				String vk = pkg.vendors.GetKey(i);
				String vv = pkg.vendors[i];
				
				if (vv.GetCount())
					s << "#if " << GetMacroConditionals(vv) << "\n";
				
				for (int j = 0; j < pkg.ifaces.GetCount(); j++) {
					String ik = pkg.ifaces.GetKey(j);
					String iv = pkg.ifaces[j];
					
					if (iv.GetCount())
						s << "#if " << GetMacroConditionals(iv) << "\n";
					
					s << "using " << vk << ik << " = " << n << ik << "T<" << a << vk << ">;\n";
					
					if (iv.GetCount())
						s << "#endif\n";
				}
				
				if (vv.GetCount())
					s << "#endif\n";
				
			}
			
			
			s	<< "\n"
				<< "END_UPP_NAMESPACE\n"
				<< "\n"
			
				<< "#endif\n";
			
			//LOG(s);
			outputs.Add(h_file, s);
		}
		
		
		
		{
			String s;
			
			s	<< "// This file have been generated automatically.\n"
				<< "// DO NOT MODIFY THIS FILE!\n\n";
				
			for(int i = 0; i < pkg.ifaces.GetCount(); i++) {
				String k = pkg.ifaces.GetKey(i);
				String v = pkg.ifaces[i];
				
				if (v.GetCount())
					s << "#if " << GetMacroConditionals(v) << "\n";
				
				String nat_this_ = "Native" + k + "&, ";
				
				s	<< "static bool " << k << "_Create(Native" + k + "*& dev);\n"
					<< "static void " << k << "_Destroy(Native" + k + "*& dev);\n"
					<< "static bool " << k << "_Initialize(" << nat_this_ << "AtomBase&, const WorldState&);\n"
					<< "static bool " << k << "_PostInitialize(" << nat_this_ << "AtomBase&);\n"
					<< "static bool " << k << "_Start(" << nat_this_ << "AtomBase&);\n"
					<< "static void " << k << "_Stop(" << nat_this_ << "AtomBase&);\n"
					<< "static void " << k << "_Uninitialize(" << nat_this_ << "AtomBase&);\n"
					<< "static bool " << k << "_Send(" << nat_this_ << "AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);\n"
					<< "static void " << k << "_Visit(" << nat_this_ << "AtomBase&, Visitor& vis);\n"
					;
				
				if (pkg.have_recv_finalize) {
					s	<< "static bool " << k << "_Recv(" << nat_this_ << "AtomBase&, int, const Packet&);\n"
						<< "static void " << k << "_Finalize(" << nat_this_ << "AtomBase&, RealtimeSourceConfig&);\n";
				}
				if (pkg.have_update) {
					s	<< "static void " << k << "_Update(" << nat_this_ << "AtomBase&, double dt);\n";
				}
				
				if (pkg.have_negotiate_format) {
					s	<< "static bool " << k << "_NegotiateSinkFormat(" << nat_this_ << "AtomBase&, LinkBase& link, int sink_ch, const ValueFormat& new_fmt);\n";
				}
				
				if (pkg.have_is_ready) {
					s	<< "static bool " << k << "_IsReady(" << nat_this_ << "AtomBase&, PacketIO& io);\n";
				}
				
				if (pkg.have_context_fns) {
					s	<< "static bool " << k << "_AttachContext(" << nat_this_ << "AtomBase& a, AtomBase& other);\n";
					s	<< "static void " << k << "_DetachContext(" << nat_this_ << "AtomBase& a, AtomBase& other);\n";
				}
				
				if (pkg.have_sound_fns) {
					s	<< "static bool " << k << "_IsDebugSoundEnabled(const Native" << k << "&, const AtomBase&);\n";
					s	<< "static String " << k << "_GetDebugSoundOutput(const Native" << k << "&, const AtomBase&);\n";
					s	<< "static int " << k << "_GetDebugSoundSeed(const Native" << k << "&, const AtomBase&);\n";
				}
				
				if (pkg.have_debug_fns) {
					s	<< "static bool " << k << "_IsDebugPrintEnabled(const Native" << k << "&, const AtomBase&);\n";
				}
				
				if (v.GetCount())
					s << "#endif\n";
				
				s << "\n";
				
			}
			
			//LOG(s);
			outputs.Add(iface_file, s);
		}
	}

	// Generate atom classes (GeneratedMinimal.h, GeneratedAudio.h, etc.)
	{
		String def_atom = "AtomLocal";
		VectorMap<String, Vector<int>> atom_list;
		int i = 0;
		for (Header& h : headers) {
			String atom = h.args.Get("HINT_PKG", def_atom);
			atom_list.GetAdd(atom).Add(i);
			i++;
		}
		
		for(int i = 0; i < atom_list.GetCount(); i++) {
			String k = atom_list.GetKey(i);
			const Vector<int>& ai = atom_list[i];
			// Generate files to uppsrc/Eon/Lib/Generated{PkgName}.{h,cpp}
			String pkg_suffix = k.Mid(4);  // Remove "Atom" prefix: AtomMinimal -> Minimal
			String genh_path = AppendFileName(eon_lib_dir, "Generated" + pkg_suffix + ".h");
			String genc_path = AppendFileName(eon_lib_dir, "Generated" + pkg_suffix + ".cpp");
			
			{
				String s;

				s	<< "#ifndef _EonLib_Generated" << pkg_suffix << "_h_\n"
					<< "#define _EonLib_Generated" << pkg_suffix << "_h_\n"
					<< "\n"
					<< "// This file is generated. Do not modify this file.\n"
					<< "\n";
				
				for(int j = 0; j < ai.GetCount(); j++) {
					const Header& h = headers[ai[j]];
					
					String cond = GetBaseConds(h.base);
					
					if (cond.GetCount())
						s << "#if " << GetMacroConditionals(cond) << "\n";
					
					s	<< "class " << h.name << " : public " << h.base << " {\n"
						<< "\n"
						<< "public:\n"
						<< "\tATOM_CTOR_(" << h.name << ", " << h.base << ")\n"
						<< "\t//ATOMTYPE(" << h.name << ")\n"
						<< "\tstatic String GetAction();\n"
						<< "\tstatic AtomTypeCls GetAtomType();\n"
						<< "\tstatic LinkTypeCls GetLinkType();\n"
						<< "\tvoid Visit(Vis& v) override;\n"
						<< "\tAtomTypeCls GetType() const override;\n"
						<< "\t\n"
						<< "};\n";
					
					if (cond.GetCount())
						s << "#endif\n";
					
					s	<< "\n";
				}
				
				s << "#endif\n";
				
				outputs.Add(genh_path, s);
			}
			
			{
				String s;

				s	<< "#include \"Lib.h\"\n"
					<< "\n"
					<< "// This file is generated. Do not modify this file.\n"
					<< "\n"
					<< "NAMESPACE_UPP\n"
					<< "\n\n";
				
				for(int j = 0; j < ai.GetCount(); j++) {
					const Header& h = headers[ai[j]];
					
					// Some error handling (possibly in bad place)
					if (h.link_type == "PIPE") {
						for(int i = 0; i < h.ins.GetCount(); i++) {
							//ASSERT(!h.ins[i]); // TODO: Fix data - PIPE types should have required ins/outs
						}
						for(int i = 0; i < h.outs.GetCount(); i++) {
							//ASSERT(!h.outs[i]); // TODO: Fix data - PIPE types should have required ins/outs
						}
					}
					
					
					String cond = GetBaseConds(h.base);
					
					if (cond.GetCount())
						s << "#if " << GetMacroConditionals(cond) << "\n";
					
					s	<< "String " << h.name << "::GetAction() {\n"
						<< "\treturn \"" << h.action << "\";\n"
						<< "}\n\n"
						<< "AtomTypeCls " << h.name << "::GetAtomType() {\n"
						<< "\tAtomTypeCls t;\n"
						<< "\tt.sub = SUB_ATOM_CLS; //" << ToUpper(h.name) << ";\n"
						<< "\tt.role = AtomRole::" << ToUpper(h.role) << ";\n";
						
					for(int i = 0; i < h.ins.GetCount(); i++) {
						String k = h.ins.GetKey(i);
						bool opt = h.ins[i];
						s << "\tt.AddIn(" << GetVD(k) << "," << (int)opt << ");\n";
					}
						
					for(int i = 0; i < h.outs.GetCount(); i++) {
						String k = h.outs.GetKey(i);
						bool opt = h.outs[i];
						s << "\tt.AddOut(" << GetVD(k) << "," << (int)opt << ");\n";
					}
					
					s	<< "\treturn t;\n"
						<< "}\n\n"
						<< "LinkTypeCls " << h.name << "::GetLinkType() {\n"
						<< "\treturn LINKTYPE(" << ToUpper(h.link_type) << ", " << ToUpper(h.link_role) << ");\n"
						<< "}\n\n"
						<< "void " << h.name << "::Visit(Vis& v) {\n"
						<< "\tVIS_THIS(" << h.base << ");\n"
						<< "}\n\n"
						<< "AtomTypeCls " << h.name << "::GetType() const {\n"
						<< "\treturn GetAtomType();\n"
						<< "}\n";
					
					if (cond.GetCount())
						s << "#endif\n";
					
					s	<< "\n\n";
				}

				s	<< "\nEND_UPP_NAMESPACE\n";

				//LOG(s);

				outputs.Add(genc_path, s);
			}
		}

		// Generate Lib.icpp with all atom registrations
		String lib_icpp_path = AppendFileName(eon_lib_dir, "Lib.icpp");
		{
			String s;
			s	<< "#include \"Lib.h\"\n"
				<< "\n\n\n"
				<< "INITBLOCK {\n"
				<< "\tusing namespace Upp;\n"
				<< "\t\n"
				<< "\t#define REGISTER_ATOM(x) VfsValueExtFactory::RegisterAtom<x>(#x);\n";

			// Generate registrations organized by package
			for(int pkg_idx = 0; pkg_idx < atom_list.GetCount(); pkg_idx++) {
				String pkg_name = atom_list.GetKey(pkg_idx);
				const Vector<int>& ai = atom_list[pkg_idx];
				s << "\t// " << pkg_name << "\n\t\n";

				for(int j = 0; j < ai.GetCount(); j++) {
					const Header& h = headers[ai[j]];
					String cond = GetBaseConds(h.base);

					if (cond.GetCount())
						s << "\t#if " << GetMacroConditionals(cond) << "\n";

					s << "\tREGISTER_ATOM(" << h.name << ");\n";

					if (cond.GetCount())
						s << "\t#endif\n";
				}
				s << "\t\n";
			}

			s	<< "\t#undef REGISTER_ATOM\n"
				<< "\t\n"
				<< "\t\n"
				<< "\t// SerialMach\n"
				<< "\t#define REGISTER_LINK(x) VfsValueExtFactory::RegisterLink<x>(#x);\n"
				<< "\tREGISTER_LINK(CustomerLink);\n"
				<< "\tREGISTER_LINK(PipeLink);\n"
				<< "\tREGISTER_LINK(PipeOptSideLink);\n"
				<< "\tREGISTER_LINK(IntervalPipeLink);\n"
				<< "\tREGISTER_LINK(ExternalPipeLink);\n"
				<< "\tREGISTER_LINK(DriverLink);\n"
				<< "\tREGISTER_LINK(MergerLink);\n"
				<< "\tREGISTER_LINK(JoinerLink);\n"
				<< "\tREGISTER_LINK(SplitterLink);\n"
				<< "\tREGISTER_LINK(PollerLink);\n"
				<< "\t#undef REGISTER_LINK\n"
				<< "\t\n"
				<< "}\n"
				<< "\n\n";

			outputs.Add(lib_icpp_path, s);
		}
	}
	
	
	if (write_actually) {
		for(int i = 0; i < outputs.GetCount(); i++) {
			String file = outputs.GetKey(i);
			String content = outputs[i];
			LOG("Writing " << file);
			
			FileOut fout(file);
			fout << content;
			fout.Close();
		}
	}
	
	
}

String InterfaceBuilder::Header::GetMacro() const {
	String s;
	bool prev_lower = false;
	for(int i = 0; i < name.GetCount(); i++) {
		int chr = name[i];
		bool lower = !IsUpper(chr);
		
		if (prev_lower && !lower)
			s.Cat('_');
		
		s.Cat(ToUpper(chr));
		
		prev_lower = lower;
	}
	return s;
}

String InterfaceBuilder::GetMacroConditionals(String cond_str) {
	String s;
	ASSERT(!cond_str.IsEmpty());
	cond_str.Replace(" ", "");
	Vector<String> ors = Split(cond_str, "|");
	int i = 0;
	int parts = 0;
	for (String o : ors) {
		if (i++) s << " || ";
		s << "(";
		Vector<String> ands = Split(o, "&");
		int j = 0;
		for (String a : ands) {
			if (j++) s << " && ";
			if (a.Left(1) == "!")
				s << "!defined flag" << a.Mid(1);
			else
				s << "defined flag" << a;
			parts++;
		}
		s << ")";
	}
	
	if (parts == 1) {
		if (cond_str.Left(1) == "!")
			return "!defined flag" + cond_str.Mid(1);
		else
			return "defined flag" + cond_str;
	}
	
	return s;
}

String InterfaceBuilder::GetVD(String vd_name) {
	int b = 0;
	for(int i = 1; i < vd_name.GetCount(); i++) {
		if (IsUpper(vd_name[i])) {
			b = i;
			break;
		}
	}
	String dev = vd_name.Left(b);
	String val = vd_name.Mid(b);
	
	return "VD(" + ToUpper(dev) + "," + ToUpper(val) + ")";
}

String InterfaceBuilder::GetMacroName(String name) {
	String s;
	bool prev_lower = false;
	for(int i = 0; i < name.GetCount(); i++) {
		int chr = name[i];
		bool lower = !IsUpper(chr);
		
		if (prev_lower && !lower)
			s.Cat('_');
		
		s.Cat(ToUpper(chr));
		
		prev_lower = lower;
	}
	return s;
}

String InterfaceBuilder::GetMacroFlags(String flags) {
	Vector<String> v = Split(flags, "_");
	String s;
	for (String vs : v) {
		if (!s.IsEmpty())
			s << " && ";
		
		if (vs.Left(1) == "!")
			s << "!defined flag" << vs.Mid(1);
		else
			s << "defined flag" << vs;
	}
	return s;
}

bool InterfaceBuilder::HasBase(String s) const {
	//DUMP(s);
	for(const Pkg& p : packages) {
		for(int i = 0; i < p.vendors.GetCount(); i++) {
			String vendor = p.vendors.GetKey(i);
			for(int j = 0; j < p.ifaces.GetCount(); j++) {
				String iface = p.ifaces.GetKey(j);
				String base = vendor + iface;
				//DUMP(base);
				if (base == s) {
					ASSERT(custom_atom_bases.Find(s) < 0);
					return true;
				}
			}
		}
	}
	
	if (custom_atom_bases.Find(s) >= 0)
		return true;
	
	DUMP(s);
	return false;
}

String InterfaceBuilder::GetBaseConds(String s) const {
	int i = custom_atom_bases.Find(s);
	if (i >= 0)
		return custom_atom_bases[i];
	
	for(const Pkg& p : packages) {
		for(int i = 0; i < p.vendors.GetCount(); i++) {
			String vendor = p.vendors.GetKey(i);
			for(int j = 0; j < p.ifaces.GetCount(); j++) {
				String iface = p.ifaces.GetKey(j);
				String base = vendor + iface;
				//DUMP(base);
				if (base == s) {
					String vcond = p.vendors[i];
					String icond = p.ifaces[j];
					vcond.Replace(" ", "");
					icond.Replace(" ", "");
					Index<String> ors;
					if (vcond.GetCount() && icond.GetCount()) {
						for (String a : Split(vcond, "|"))
							for (String b : Split(icond, "|"))
								ors.FindAdd(a + "&" + b);
					}
					else if (vcond.GetCount()) {
						for (String cond : Split(vcond, "|"))
							ors.FindAdd(cond);
					}
					else if (icond.GetCount()) {
						for (String cond : Split(icond, "|"))
							ors.FindAdd(cond);
					}
					
					SortIndex(ors, StdLess<String>());
					
					String s;
					for(int i = 0; i < ors.GetCount(); i++) {
						if (i) s.Cat('|');
						s.Cat(ors[i]);
					}
					return s;
				}
			}
		}
	}
	
	NEVER();
	Panic("error");
	return String();
}

END_UPP_NAMESPACE
