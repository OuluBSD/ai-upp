#ifndef _Eon_Fn_h_
#define _Eon_Fn_h_


template <class T>
void MemSwap(T& a, T& b) {
	uint8 tmp[sizeof(T)];
	MemoryCopy((void*)tmp, (void*)&a, sizeof(T));
	MemoryCopy((void*)&a, (void*)&b, sizeof(T));
	MemoryCopy((void*)&b, (void*)tmp, sizeof(T));
}


template <class T>
void EndianSwapT(T& o) {
	static const int bytes = sizeof(T);
	static const int bytes_2 = bytes / 2;
	if (bytes_2 > 0) {
		byte* a = (byte*)&o;
		byte* b = a + bytes - 1;
		for(int i = 0; i < bytes_2; i++) {
			byte c = *a;
			*a++ = *b;
			*b-- = c;
		}
	}
}


#endif
