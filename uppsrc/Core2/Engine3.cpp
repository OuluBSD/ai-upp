#include "Core.h"

// Keeping 3 separate Engine files for historical git-log reasons

NAMESPACE_UPP

#if 0

void Engine::Main(String script_content, String script_file, Value args_, MachineVerifier* ver, bool dbg_ref_visits, uint64 dbg_ref) {
	SetCoutLog();
	
	if (script_content.IsEmpty() && script_file.IsEmpty()) {
		LOG("No script file given");
		return;
	}
	
	//if (dbg_ref)
	//	BreakRefAdd(dbg_ref);
	
	ValueMap args = args_;
	double time_limit = args.Get("MACHINE_TIME_LIMIT", 0);
	
	{
		Engine& mach = *this;
		
		if (ver)
			ver->Attach(mach);
		
		//if (dbg_ref_visits)
		//	SetDebugRefVisits();
		
		//RuntimeDiagnostics::Static().SetRoot(mach); // TODO not global -> VfsValue under Machine
		
	    #ifdef flagSTDEXC
	    try {
	    #endif
			bool fail = false;
			{
				TODO
				#if 0
				if (mach.IsStarted()) {
					Ptr<Eon::ScriptLoader> script	= mach.FindAdd<Eon::ScriptLoader>();
				    
				    mach.FindAdd<PacketTracker>();
				}
				
				Ptr<Eon::ScriptLoader> script	= mach.Find<Eon::ScriptLoader>();
				if (!script) {
					LOG("No ScriptLoader added to machine and the machine is already started");
					return;
				}
				#endif
				
				String path = RealizeEonFile(script_file);
				
				String script_str;
				if (script_content.IsEmpty())
					script_str = LoadFile(path);
				else
					script_str = script_content;
				
				if (script_str.IsEmpty()) {
					LOG("No script file in " << path);
					return;
				}
				for(int i = 0; i < args.GetCount(); i++) {
					String key = "${" + args.GetKey(i).ToString() + "}";
					String value = args[i].ToString();
					value = EscapeString(value);
					script_str.Replace(key, value);
				}
				LOG(script_str);
				
				TODO
				#if 0
		        script->PostLoadString(script_str);
		        #endif
		    }
		        
		    if (!fail) {
		        if (!mach.IsStarted())
					fail = !mach.Start();
		    }
		    
		    if (!fail) {
			    int iter = 0;
			    TimeStop t, total;
			    double sleep_dt_limit = 1.0 / 300.0;
			    while (mach.IsRunning()) {
			        double dt = ResetSeconds(t);
			        mach.Update(dt);
			        
			        if (dt < sleep_dt_limit)
						Sleep(1);
			        
			        double total_seconds = total.Seconds();
			        if (time_limit > 0 && total_seconds >= time_limit)
			            mach.SetNotRunning();
			    }
			    
			    //RuntimeDiagnostics::Static().CaptureSnapshot();
		    }
		#ifdef flagSTDEXC
	    }
	    catch (Exc e) {
	        LOG("error: " << e);
	        Exit(1);
	    }
	    #endif
	    
	    mach.Stop();
	    mach.Clear();
	}
}

#endif

END_UPP_NAMESPACE
