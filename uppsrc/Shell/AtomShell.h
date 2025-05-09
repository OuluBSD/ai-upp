#ifndef _Shell_AtomShell_h_
#define _Shell_AtomShell_h_


void DefaultStartup();
void DefaultRunner(bool main_loop, String app_name, String override_eon_file="", VectorMap<String,Value>* extra_args=0, const char* extra_str=0);
void DefaultRunnerStop();
bool DefaultInitializer(bool skip_eon_file);
void DefaultSerialInitializer();
void DefaultSerialInitializer0(bool skip_eon_file=false);
void DefaultSerialInitializerInternalEon();
void DebugMainLoop();
void DebugMainLoop(Machine& mach, bool (*fn)(void*)=0, void* arg=0);

struct SerialLoaderBase {
	
	virtual ~SerialLoaderBase() {}
	virtual String LoadFile(String file_path) = 0;
	
};

struct SerialLoaderFactory {
	typedef SerialLoaderBase*(*NewFn)();
	struct Loader : Moveable<Loader> {
		NewFn fn;
		
	};
	
	static VectorMap<String,Loader>& GetLoaders() {static VectorMap<String,Loader> l; return l;}
	
	
	template <class T>
	static SerialLoaderBase* New() {return new T();}
	
	
	template <class T>
	static void Register(String file_ext) {
		Loader& l = GetLoaders().Add(file_ext);
		l.fn = &New<T>;
	}
	
	
	static String LoadFile(String file_path);
};





#endif
