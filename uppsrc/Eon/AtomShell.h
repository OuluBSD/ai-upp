#ifndef _Eon_AtomShell_h_
#define _Eon_AtomShell_h_


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

void DefaultStartup();
void DefaultSerialInitializer();
void DefaultSerialInitializerInternalEon();

#endif
