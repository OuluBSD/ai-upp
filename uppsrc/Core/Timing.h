#ifndef _Core_Timing_h_
#define _Core_Timing_h_

struct TimingKey : Moveable<TimingKey> {
	String  label;
	String  context;
	String  callstack;
	String  thread_name;
	uint64  thread_id;
	byte    category = 0;

	TimingKey();
	TimingKey(byte category, const String& label, const String& context, uint64 thread_id, const String& callstack, const String& thread_name);

	bool operator<(const TimingKey& b) const;
	bool operator==(const TimingKey& b) const;
	hash_t GetHashValue() const;
	String ToString() const;
	ValueMap ToValueMap() const;
};

enum TimingEventCategory {
	TIMING_EVENT_SCOPE           = 1,
	TIMING_EVENT_CONTEXT         = 2,
	TIMING_EVENT_MARKER          = 3,
	TIMING_EVENT_MUTEX_WAIT      = 4,
	TIMING_EVENT_MUTEX_HOLD      = 5,
	TIMING_EVENT_SEMAPHORE_WAIT  = 6,
	TIMING_EVENT_SPIN_WAIT       = 7,
	TIMING_EVENT_MEMORY_ALLOC    = 8,
	TIMING_EVENT_MEMORY_FREE     = 9,
	TIMING_EVENT_METADATA_UPDATE = 10,
	TIMING_EVENT_LOCK            = TIMING_EVENT_MUTEX_WAIT,
	TIMING_EVENT_MEMORY          = TIMING_EVENT_MEMORY_ALLOC,
};

struct TimingCallstack : Moveable<TimingCallstack> {
	Vector<uintptr_t> frame;

	void  Clear();
	bool  IsEmpty() const             { return frame.IsEmpty(); }
	void  Capture(int skip = 0, int depth = 32);
	String ToString() const;
	ValueMap ToValueMap() const;
};

struct TimingSample : Moveable<TimingSample> {
	TimingKey       key;
	dword           elapsed = 0;
	int64           value = 0;
	int             nesting = 0;
	uint64          sequence = 0;
	uint64          dropped_before = 0;

	String ToString() const;
	ValueMap ToValueMap() const;
};

struct TimingRecord : Moveable<TimingRecord> {
	TimingKey       key;
	int             count = 0;
	dword           total_time = 0;
	int64           total_value = 0;
	int64           min_value = 0;
	int64           max_value = 0;
	dword           min_time = 0;
	dword           max_time = 0;
	int             max_nesting = 0;
	uint64          first_sequence = 0;
	uint64          last_sequence = 0;
	uint64          dropped_before = 0;

	String ToString() const;
	ValueMap ToValueMap() const;
};

struct TimingStorageStats : Moveable<TimingStorageStats> {
	int    event_capacity = 0;
	int    event_count = 0;
	uint64 sequence = 0;
	uint64 overwritten = 0;
	uint64 dropped = 0;
	uint64 retained_from_sequence = 0;
	uint64 retained_to_sequence = 0;
	int64  estimated_event_bytes = 0;

	String ToString() const;
	ValueMap ToValueMap() const;
};

class TimingStore {
public:
	TimingStore();
	~TimingStore();

	void  Clear();

	void  SetEnabled(bool b);
	bool  IsEnabled() const;

	void  SetCaptureCallstack(bool b);
	bool  IsCaptureCallstack() const;

	void  SetKeepTimeline(bool b);
	bool  IsKeepTimeline() const;

	void  SetEventCapacity(int capacity);
	int   GetEventCapacity() const;
	void  SetMaxStorageBytes(int64 bytes);

	void  SetLockEvents(bool b);
	bool  IsLockEvents() const;

	void  SetMemoryEvents(bool b);
	bool  IsMemoryEvents() const;

	void  Record(const char *label, const String& context, dword elapsed, int nesting);
	void  Record(byte category, const char *label, int context_id, dword elapsed, int nesting, int64 value = 0);
	void  Marker(const char *label, int context_id);
	int   InternContext(const String& context);

	Vector<TimingRecord> GetRecords() const;
	Vector<TimingSample> GetTimeline() const;
	TimingStorageStats GetStorageStats() const;
	ValueMap GetValueMap(bool include_timeline = false) const;
	String GetJson(bool include_timeline = false, bool pretty = false) const;
	String Dump() const;

	int GetRecordCount() const;
	int GetTimelineCount() const;

private:
#ifdef flagTIMING
	struct RawEvent {
		const char *label = NULL;
		uint64      thread_id = 0;
		uint64      sequence = 0;
		uint64      dropped_before = 0;
		dword       elapsed = 0;
		int64       value = 0;
		int         nesting = 0;
		int         context_id = 0;
		int         callstack_id = 0;
		const char *thread_name = NULL;
		byte        category = 0;
	};

	mutable StaticMutex               mutex;
	Vector<RawEvent>                  events;
	int                               event_pos = 0;
	int                               event_count = 0;
	uint64                            overwritten = 0;
	uint64                            dropped = 0;
	Vector<String>                    contexts;
	Vector<String>                    callstacks;
	bool                              enabled = true;
	bool                              capture_callstack = false;
	bool                              keep_timeline = false;
	bool                              lock_events = true;
	bool                              memory_events = true;
	uint64                            sequence = 0;

	String GetContextText(int context_id) const;
	String GetCallstackText(int callstack_id) const;
	int    InternCallstack(const String& callstack);
	int    GetRetainedBegin() const;
	void   CopyRetainedEvents(Vector<RawEvent>& out) const;
	TimingSample MakeSample(const RawEvent& ev) const;
	void   AddRecord(VectorMap<TimingKey, TimingRecord>& records, const RawEvent& ev) const;
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

	void  SetEventCapacity(int capacity);
	int   GetEventCapacity() const;
	void  SetMaxStorageBytes(int64 bytes);

	void  SetLockEvents(bool b);
	bool  IsLockEvents() const;

	void  SetMemoryEvents(bool b);
	bool  IsMemoryEvents() const;

	void  SetContext(const String& context);
	String GetContext() const;
	void  SetThreadName(const char *name);
	String GetThreadName() const;
	void  BeginContext(const char *label, const String& metadata = String());
	void  EndContext(const char *label, const String& metadata = String());
	void  SetMetadata(const char *label, const String& metadata = String());
	void  Marker(const char *label);

	void  Clear();
	void  Record(const char *label, dword elapsed, int nesting);
	void  RecordLockWait(const char *label, dword elapsed);
	void  RecordLockHold(const char *label, dword elapsed);
	void  RecordSemaphoreWait(const char *label, dword elapsed);
	void  RecordSpinWait(const char *label, dword elapsed);
	void  RecordMemoryAlloc(size_t size);
	void  RecordMemoryFree(size_t size);

	static bool IsProfilingSuppressed();
	static bool IsLockEventCollectionActive();
	static bool IsMemoryEventCollectionActive();

	Vector<TimingRecord> GetRecords() const;
	Vector<TimingSample> GetTimeline() const;
	TimingStorageStats GetStorageStats() const;
	ValueMap GetValueMap(bool include_timeline = false) const;
	String GetJson(bool include_timeline = false, bool pretty = false) const;
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
	const char *prev_label = NULL;
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

class TimingContextInterval {
public:
	TimingContextInterval(const char *label, const String& metadata = String());
	~TimingContextInterval();

private:
	const char *label = NULL;
	String      metadata;
	bool        active = false;
};

#endif
