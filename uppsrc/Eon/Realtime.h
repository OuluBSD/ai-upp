#ifndef _Eon_Realtime_h_
#define _Eon_Realtime_h_


class RealtimeStream : public Pte<RealtimeStream>
{
public:
	virtual ~RealtimeStream() {}
	virtual void Clear() = 0;
	virtual String GetLastError() const {return String();}
};


#endif
