#ifndef _Core_Timing_h_
#define _Core_Timing_h_

struct TimingKey : Moveable<TimingKey> {
	String  label;
	String  context;
	String  callstack;
	uint64  thread_id;

	TimingKey();
	TimingKey(const String& label, const String& context, uint64 thread_id, const String& callstack);

	bool operator<(const TimingKey& b) const;
	bool operator==(const TimingKey& b) const;
	hash_t GetHashValue() const;
	String ToString() const;
};

struct TimingCallstack : Moveable<TimingCallstack> {
	Vector<uintptr_t> frame;

	void  Clear();
	bool  IsEmpty() const             { return frame.IsEmpty(); }
	void  Capture(int skip = 0, int depth = 32);
	String ToString() const;
};

struct TimingSample : Moveable<TimingSample> {
	TimingKey       key;
	dword           elapsed = 0;
	int             nesting = 0;
	uint64          sequence = 0;

	String ToString() const;
};

struct TimingRecord : Moveable<TimingRecord> {
	TimingKey       key;
	int             count = 0;
	dword           total_time = 0;
	dword           min_time = 0;
	dword           max_time = 0;
	int             max_nesting = 0;
	uint64          first_sequence = 0;
	uint64          last_sequence = 0;

	String ToString() const;
};

class TimingStore {
public:
	TimingStore();

	void  Clear();

	void  SetEnabled(bool b);
	bool  IsEnabled() const;

	void  SetCaptureCallstack(bool b);
	bool  IsCaptureCallstack() const;

	void  SetKeepTimeline(bool b);
	bool  IsKeepTimeline() const;

	void  Record(const char *label, const String& context, dword elapsed, int nesting);

	Vector<TimingRecord> GetRecords() const;
	Vector<TimingSample> GetTimeline() const;
	String Dump() const;

	int GetRecordCount() const;
	int GetTimelineCount() const;

private:
#ifdef flagTIMING
	mutable StaticMutex               mutex;
	VectorMap<TimingKey, TimingRecord> records;
	Vector<TimingSample>              timeline;
	bool                              enabled = true;
	bool                              capture_callstack = false;
	bool                              keep_timeline = false;
	uint64                            sequence = 0;
#endif
};

class TimingManager {
public:
	static TimingManager& Global();

	void  Activate(bool b);
	bool  IsActive() const;

	void  SetCaptureCallstack(bool b);
	bool  IsCaptureCallstack() const;

	void  SetKeepTimeline(bool b);
	bool  IsKeepTimeline() const;

	void  SetContext(const String& context);
	String GetContext() const;

	void  Clear();
	void  Record(const char *label, dword elapsed, int nesting);

	Vector<TimingRecord> GetRecords() const;
	Vector<TimingSample> GetTimeline() const;
	String Dump() const;

private:
#ifdef flagTIMING
	TimingStore store;
#endif
};

class TimingScope {
public:
	TimingScope(const char *label);
	~TimingScope();

private:
	const char *label = NULL;
	dword       start_time = 0;
	bool        active = false;
};

class TimingContextScope {
public:
	TimingContextScope(const String& context);
	~TimingContextScope();

private:
	String prev;
	bool   active = false;
};

#endif
