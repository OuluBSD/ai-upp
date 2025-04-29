#ifndef _Eon_PacketTracker_h_
#define _Eon_PacketTracker_h_


class PacketTracker 
{
	PacketId id_counter = 1;
	
	
protected:
    bool Initialize();
    void Start();
    void Update(double dt);
    void Stop();
    void Uninitialize();
    
    #if HAVE_PACKETTRACKER
	void Track0(TrackerInfo info, PacketValue& p);
	void Checkpoint0(TrackerInfo info, PacketValue& p);
	void StopTracking0(TrackerInfo info, PacketValue& p);
	#endif
	
	static PacketTracker*& active_tracker() {static PacketTracker* p; return p;}
public:
    PacketTracker();
	void Visit(Vis& vis) {}
	
	#if HAVE_PACKETTRACKER
	static void Track(TrackerInfo info, Packet& p) {Track(info, *p);}
	static void Checkpoint(TrackerInfo info, Packet& p) {Checkpoint(info, *p);}
	static void StopTracking(TrackerInfo info, Packet& p) {StopTracking(info, *p);}
	
	static void Track(TrackerInfo info, PacketValue& p) {if (active_tracker()) active_tracker()->Track0(info,p);}
	static void Checkpoint(TrackerInfo info, PacketValue& p) {if (active_tracker()) active_tracker()->Checkpoint0(info,p);}
	static void StopTracking(TrackerInfo info, PacketValue& p) {if (active_tracker()) active_tracker()->StopTracking0(info,p);}
	#endif
};

#undef RTTI_CTX_SYS


#endif
