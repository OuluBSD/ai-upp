#include "Eon.h"
//#include <EcsMach/EcsMach.h>

NAMESPACE_UPP


using ObjMap = VectorMap<String,Value>;
MAKE_STATIC(ObjMap, __def_args)
MAKE_STATIC(String, eon_script)
MAKE_STATIC(String, eon_file)


dword active_shell_mode;

void DebugMain(bool main_loop, String script_content, String script_file, VectorMap<String,Value>& args, bool dbg_ref_visits=false, uint64 dbg_ref=0);

bool IsShellMode(ShellMode m) {
	return active_shell_mode & (dword)m;
}

void SetShellMode(ShellMode m) {
	active_shell_mode |= (dword)m;
}

bool IsEmptyShellMode() {
	return active_shell_mode == 0;
}

bool DefaultInitializer(bool skip_eon_file) {
	SetCoutLog();
	CommandLineArguments cmd;
	DaemonBase& daemon = DaemonBase::Single();
	
	// Script interpreter platform
	cmd.AddArg('e', "The path for the eon file", true, "path");
	
	// Development platform (remote connection)
	cmd.AddArg('l', "Listen to connections", false);
	cmd.AddArg('p', "Port", true, "integer");
	cmd.AddArg('v', "Verbose", false);
	
	// Parse command line
	if (!cmd.Parse()) {
		cmd.PrintHelp();
		return false;
	}
	
	// Get rest of the command line arguments
	__def_args <<= cmd.GetVariables();
	
	// Arguments for remote connection
	TODO // include enet or virtualize ?
	#if 0
	if (cmd.IsArg('l')) {
		daemon.Add("EnetServer");
		daemon.Add("Ecs");
		EnetServiceServer::SetVerbose(cmd.IsArg('v'));
		EnetServiceServer::port_arg = StrInt(cmd.GetArg('p'));
		if (EnetServiceServer::port_arg) {
			LOG("User gave listening port " << (int)EnetServiceServer::port_arg);
		}
		SetShellMode(SHELLMODE_REMOTE);
	}
	#endif
	
	// Arguments for script interpreter platform
	const auto& inputs = cmd.GetInputs();
	for(const auto& in : inputs) {
		if (in.key == 'e') {
			eon_file = in.value;
			SetShellMode(SHELLMODE_INTERPRETER);
		}
	}
	
	// Remote connection startup
	if (IsShellMode(SHELLMODE_REMOTE)) {
		// Start daemon in a separate thread
		if (!daemon.Init()) {
			LOG("DefaultInitializer: error: couldn't start daemon");
			return false;
		}
		daemon.RunInThread();
	}
	
	// Script interpreter platform startup
	if (IsShellMode(SHELLMODE_INTERPRETER)) {
		if (!skip_eon_file && eon_file.IsEmpty()) {
			cmd.PrintHelp();
			LOG("");
			LOG("\te.g. -e play_audio_file.eon -MACHINE_TIME_LIMIT=3 -FILE=/home/user/some.mp3");
			return false;
		}
		
		eon_file = RealizeEonFile(eon_file);
		
		if (GetFileExt(eon_file) != ".eon") {
			eon_script = SerialLoaderFactory::LoadFile(eon_file);
			if (eon_script.IsEmpty())
				return false;
		}
	}
	
	// Error handling
	if (IsEmptyShellMode()) {
		LOG("DefaultInitializer: error: no enough program arguments to do anything");
		return false;
	}
	
	
	return true;
}


size_t break_addr = 0;


void DefaultSerialInitializer() {DefaultSerialInitializer0(false);}

void DefaultSerialInitializer0(bool skip_eon_file) {
	
	
	SetCoutLog();
	
	if (0)
		break_addr = 0x1;
	
	
	if (!DefaultInitializer(skip_eon_file))
		GetActiveMachine().SetFailed("Default serial initializer failed");
	
}

void DefaultSerialInitializerInternalEon() {
	DefaultSerialInitializer0(true);
}

void DefaultRunner(bool main_loop, String app_name, String override_eon_file, VectorMap<String,Value>* extra_args, const char* extra_str) {
	
	if (!override_eon_file.IsEmpty())
		eon_file = override_eon_file;
	
	if (IsShellMode(SHELLMODE_INTERPRETER)) {
		LOG("Eon-file path: " << eon_file);
	}
	
	if (extra_args) {
		for(int i = 0; i < extra_args->GetCount(); i++)
			__def_args.GetAdd(
				extra_args->GetKey(i),
				(*extra_args)[i]
			);
	}
	
	if (extra_str) {
		Vector<String> args = Split(extra_str, ";");
		for (String& arg : args) {
			int i = arg.Find("=");
			if (i >= 0) {
				String a = arg.Left(i);
				String b = arg.Mid(i+1);
				__def_args.Add(a, b);
			}
		}
	}
	
	//break_addr = 1;
	
	if (!break_addr)
		DebugMain(main_loop, eon_script, eon_file, __def_args);
	else
		DebugMain(main_loop, eon_script, eon_file, __def_args, 1, break_addr);
}

void DefaultStartup() {
	LOG("<-- Startup -->");
}




int __dbg_time_limit;




void DebugMainLoop() {
	DebugMainLoop(GetActiveMachine());
}

void DebugMainLoop(Machine& mach, bool (*fn)(void*), void* arg) {
	
	
	int iter = 0;
    TimeStop t, total;
    double sleep_dt_limit = 0.001;
    int fast_iter = 0;
    while (mach.IsRunning() && !Thread::IsShutdownThreads()) {
        double dt = ResetSeconds(t);
        
        mach.Update(dt);
        
        if (fn)
            fn(arg);
        
        if (dt < sleep_dt_limit && fast_iter > 5) {
			Sleep(1);
			fast_iter = 0;
        }
        else fast_iter++;
        
        double total_seconds = total.Seconds();
        if (__dbg_time_limit > 0 && total_seconds >= __dbg_time_limit)
            mach.SetNotRunning();
    }
    
    //RuntimeDiagnostics::Static().CaptureSnapshot();
}

void DebugMain(bool main_loop, String script_content, String script_file, VectorMap<String,Value>& args, bool dbg_ref_visits, uint64 dbg_ref) {
	
	
	SetCoutLog();
	
	if (IsShellMode(SHELLMODE_INTERPRETER)) {
		if (script_content.IsEmpty() && script_file.IsEmpty()) {
			LOG("No script file given");
			return;
		}
	}
	
	__dbg_time_limit = args.Get("MACHINE_TIME_LIMIT", 0);
	
	{
		Machine& mach = GetActiveMachine();
		
		//RuntimeDiagnostics::Static().SetRoot(mach);
		
	    #ifdef flagSTDEXC
	    try {
	    #endif
			bool fail = false;
			{
				if (!mach.IsStarted()) {
					RegistrySystemPtr reg	= mach.FindAdd<RegistrySystem>();
					//LoopStorePtr ls			= mach.FindAdd<LoopStore>();
					//AtomStorePtr as			= mach.FindAdd<AtomStore>();
				    Ptr<AtomSystem> asys		= mach.FindAdd<AtomSystem>();
				    Ptr<ScriptLoader> script	= mach.FindAdd<ScriptLoader>();
					
					#ifdef flagGUI
				    Gu::GuboSystemPtr gubo	= mach.FindAdd<Gu::GuboSystem>();
				    #endif
				    
				    #if IS_UPP_CORE && defined flagGUI
				    WindowSystemPtr win		= mach.FindAdd<WindowSystem>();
				    #endif
				    
				    mach.FindAdd<PacketTracker>();
				}
				
				Ptr<LoopStore> ls			= mach.Find<LoopStore>();
				if (!ls) {
					LOG("No LoopStore added to machine and the machine is already started");
					return;
				}
				
				
				Ptr<Eon::ScriptLoader> script	= mach.Find<Eon::ScriptLoader>();
				if (!script) {
					LOG("No ScriptLoader added to machine and the machine is already started");
					return;
				}
				
				TODO
				LoopPtr root; // = ls->GetRoot();
				
				if (IsShellMode(SHELLMODE_INTERPRETER)) {
					String path = RealizeEonFile(script_file);
					
					String script_str;
					if (script_content.IsEmpty()) {
						script_str = LoadFile(path);
						if (script_str.IsEmpty()) {
							LOG("Empty file in " << path);
							return;
						}
					}
					else
						script_str = script_content;
					
					if (script_str.IsEmpty()) {
						LOG("No script");
						return;
					}
					for(int i = 0; i < args.GetCount(); i++) {
						String key = "${" + args.GetKey(i) + "}";
						String value = args[i].ToString();
						value = EscapeString(value);
						script_str.Replace(key, value);
					}
					LOG(script_str);
				
					script->PostLoadString(script_str);
				}
		    }
		    
		    if (!fail) {
		        if (!mach.IsStarted())
					fail = !mach.Start();
		    }
		    
		    if (!fail) {
		        if (main_loop)
					DebugMainLoop();
		        else
		            Machine::WhenUserProgram();
		    }
		#ifdef flagSTDEXC
	    }
	    catch (Exc e) {
	        LOG("error: " << e);
	        Exit(1);
	    }
	    #endif
	    
	    if (main_loop) {
		    mach.Stop();
		    mach.Clear();
	    }
	}
    
}

void DefaultRunnerStop() {
	
	Machine& mach = GetActiveMachine();
	mach.SetNotRunning();
	mach.Stop();
	mach.Clear();
}


String SerialLoaderFactory::LoadFile(String file_path) {
	Cout() << "SerialLoaderFactory::LoadFile" << file_path << "\n";
	const auto& l = GetLoaders();
	for(int i = 0; i < l.GetCount(); i++) {
		Cout() << i << ": " << l.GetKey(i) << "\n";
	}
	String ext = GetFileExt(file_path);
	int i = l.Find(ext);
	if (i < 0) {
		LOG("SerialLoaderFactory: error: no loader for file extension: " << ext);
		return String();
	}
	const Loader& el = l[i];
	One<SerialLoaderBase> loader = el.fn();
	return loader->LoadFile(file_path);
}

END_UPP_NAMESPACE

