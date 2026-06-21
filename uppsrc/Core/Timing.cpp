#include "Core.h"

#ifdef PLATFORM_POSIX
#include <execinfo.h>
#include <vector>
#endif

namespace Upp {

namespace {

#ifdef flagTIMING
	thread_local String t_timing_context;
	thread_local int    t_timing_context_id = 0;
	thread_local int    t_timing_nesting = 0;
	thread_local int    t_timing_suppressed = 0;
	thread_local const char *t_timing_scope_label = NULL;
	thread_local const char *t_timing_thread_name = NULL;
	std::atomic<bool>   s_timing_active;
	std::atomic<bool>   s_timing_lock_events;
	std::atomic<bool>   s_timing_memory_events;

	enum {
		TIMING_DEFAULT_EVENT_CAPACITY = 65536,
	};

	struct TimingInternalGuard {
		TimingInternalGuard()  { ++t_timing_suppressed; }
		~TimingInternalGuard() { --t_timing_suppressed; }
	};
#endif

uint64 GetTimingThreadId()
{
#ifdef PLATFORM_WIN32
	return (uint64)GetCurrentThreadId();
#else
	return (uint64)(uintptr_t)pthread_self();
#endif
}

String FormatTimingFrame(uintptr_t frame)
{
	return Format64Hex((uint64)frame);
}

String TimingCategoryName(byte category)
{
	switch(category) {
	case TIMING_EVENT_SCOPE:   return "scope";
	case TIMING_EVENT_CONTEXT: return "context";
	case TIMING_EVENT_MARKER:  return "marker";
	case TIMING_EVENT_MUTEX_WAIT:      return "mutex_wait";
	case TIMING_EVENT_MUTEX_HOLD:      return "mutex_hold";
	case TIMING_EVENT_SEMAPHORE_WAIT:  return "semaphore_wait";
	case TIMING_EVENT_SPIN_WAIT:       return "spin_wait";
	case TIMING_EVENT_MEMORY_ALLOC:    return "memory_alloc";
	case TIMING_EVENT_MEMORY_FREE:     return "memory_free";
	case TIMING_EVENT_METADATA_UPDATE: return "metadata_update";
	default:                   return "unknown";
	}
}

} // namespace

hash_t TimingKey::GetHashValue() const
{
	CombineHash h;
	h.Do(category);
	h.Do(label);
	h.Do(context);
	h.Do(callstack);
	h.Do(thread_name);
	h.Do(thread_id);
	return h;
}

TimingKey::TimingKey()
	: thread_id(0)
{}

TimingKey::TimingKey(byte category, const String& label, const String& context, uint64 thread_id, const String& callstack, const String& thread_name)
	: label(label), context(context), callstack(callstack), thread_name(thread_name), thread_id(thread_id), category(category)
{}

bool TimingKey::operator<(const TimingKey& b) const
{
	if(category != b.category)
		return category < b.category;
	if(thread_id != b.thread_id)
		return thread_id < b.thread_id;
	if(thread_name != b.thread_name)
		return thread_name < b.thread_name;
	if(context != b.context)
		return context < b.context;
	if(label != b.label)
		return label < b.label;
	return callstack < b.callstack;
}

bool TimingKey::operator==(const TimingKey& b) const
{
	return category == b.category && thread_id == b.thread_id && context == b.context &&
	       label == b.label && callstack == b.callstack && thread_name == b.thread_name;
}

String TimingKey::ToString() const
{
	String s;
	s << "category=" << TimingCategoryName(category)
	  << " thread=" << Format64Hex(thread_id);
	if(!thread_name.IsEmpty())
		s << " thread_name=" << thread_name;
	if(!context.IsEmpty())
		s << " context=" << context;
	if(!label.IsEmpty())
		s << " label=" << label;
	if(!callstack.IsEmpty())
		s << " callstack=" << callstack;
	return s;
}

ValueMap TimingKey::ToValueMap() const
{
	ValueMap out;
	out.Add("category", (int)category);
	out.Add("category_name", TimingCategoryName(category));
	out.Add("label", label);
	out.Add("context", context);
	out.Add("callstack", callstack);
	out.Add("thread_id", Format64Hex(thread_id));
	out.Add("thread_name", thread_name);
	return out;
}

void TimingCallstack::Clear()
{
	frame.Clear();
}

void TimingCallstack::Capture(int skip, int depth)
{
	Clear();
#ifdef flagTIMING
#ifdef PLATFORM_WIN32
	Vector<void *> tmp;
	tmp.SetCount(max(depth, 0));
	USHORT count = CaptureStackBackTrace(skip + 1, depth, (PVOID *)~tmp, NULL);
	tmp.SetCount(count);
	frame.SetCount(count);
	for(int i = 0; i < count; ++i)
		frame[i] = (uintptr_t)tmp[i];
#elif defined(PLATFORM_POSIX)
	::std::vector<void *> tmp(max(depth, 0));
	int count = backtrace(tmp.data(), depth);
	if(count > skip + 1) {
		frame.SetCount(count - skip - 1);
		for(int i = skip + 1; i < count; ++i)
			frame[i - skip - 1] = (uintptr_t)tmp[i];
	}
#else
	(void)skip;
	(void)depth;
#endif
#else
	(void)skip;
	(void)depth;
#endif
}

String TimingCallstack::ToString() const
{
	String s;
	for(int i = 0; i < frame.GetCount(); ++i) {
		if(i)
			s << '|';
		s << FormatTimingFrame(frame[i]);
	}
	return s;
}

ValueMap TimingCallstack::ToValueMap() const
{
	ValueMap out;
	ValueArray frames;
	for(int i = 0; i < frame.GetCount(); ++i)
		frames.Add(FormatTimingFrame(frame[i]));
	out.Add("frames", frames);
	out.Add("text", ToString());
	return out;
}

String TimingSample::ToString() const
{
	String s;
	s << key.ToString()
	  << " elapsed=" << AsString(elapsed)
	  << " value=" << AsString(value)
	  << " nesting=" << AsString(nesting)
	  << " seq=" << AsString(sequence);
	if(dropped_before)
		s << " dropped_before=" << AsString(dropped_before);
	return s;
}

ValueMap TimingSample::ToValueMap() const
{
	ValueMap out;
	out.Add("key", key.ToValueMap());
	out.Add("category", (int)key.category);
	out.Add("category_name", TimingCategoryName(key.category));
	out.Add("label", key.label);
	out.Add("context", key.context);
	out.Add("thread_id", Format64Hex(key.thread_id));
	out.Add("thread_name", key.thread_name);
	out.Add("callstack", key.callstack);
	out.Add("elapsed", (int64)elapsed);
	out.Add("value", value);
	out.Add("nesting", nesting);
	out.Add("sequence", (int64)sequence);
	out.Add("dropped_before", (int64)dropped_before);
	return out;
}

String TimingRecord::ToString() const
{
	String s;
	s << key.ToString()
	  << " count=" << AsString(count)
	  << " total=" << AsString(total_time)
	  << " value=" << AsString(total_value)
	  << " min_value=" << AsString(min_value)
	  << " max_value=" << AsString(max_value)
	  << " min=" << AsString(min_time)
	  << " max=" << AsString(max_time)
	  << " nesting=" << AsString(max_nesting)
	  << " first=" << AsString(first_sequence)
	  << " last=" << AsString(last_sequence);
	if(dropped_before)
		s << " dropped_before=" << AsString(dropped_before);
	return s;
}

ValueMap TimingRecord::ToValueMap() const
{
	ValueMap out;
	out.Add("key", key.ToValueMap());
	out.Add("category", (int)key.category);
	out.Add("category_name", TimingCategoryName(key.category));
	out.Add("label", key.label);
	out.Add("context", key.context);
	out.Add("thread_id", Format64Hex(key.thread_id));
	out.Add("thread_name", key.thread_name);
	out.Add("callstack", key.callstack);
	out.Add("count", count);
	out.Add("total_time", (int64)total_time);
	out.Add("total_value", total_value);
	out.Add("min_time", (int64)min_time);
	out.Add("max_time", (int64)max_time);
	out.Add("min_value", min_value);
	out.Add("max_value", max_value);
	out.Add("max_nesting", max_nesting);
	out.Add("first_sequence", (int64)first_sequence);
	out.Add("last_sequence", (int64)last_sequence);
	out.Add("dropped_before", (int64)dropped_before);
	return out;
}

String TimingStorageStats::ToString() const
{
	String s;
	s << "events=" << event_count << "/" << event_capacity
	  << " sequence=" << AsString(sequence)
	  << " retained=" << AsString(retained_from_sequence) << ".." << AsString(retained_to_sequence)
	  << " overwritten=" << AsString(overwritten)
	  << " dropped=" << AsString(dropped)
	  << " estimated_event_bytes=" << AsString(estimated_event_bytes);
	return s;
}

ValueMap TimingStorageStats::ToValueMap() const
{
	ValueMap out;
	out.Add("event_capacity", event_capacity);
	out.Add("event_count", event_count);
	out.Add("sequence", (int64)sequence);
	out.Add("overwritten", (int64)overwritten);
	out.Add("dropped", (int64)dropped);
	out.Add("retained_from_sequence", (int64)retained_from_sequence);
	out.Add("retained_to_sequence", (int64)retained_to_sequence);
	out.Add("estimated_event_bytes", estimated_event_bytes);
	return out;
}

TimingStore::TimingStore()
{
#ifdef flagTIMING
#ifdef flagTIMING_DEFAULT_OFF
	enabled = false;
#else
	enabled = true;
#endif
	capture_callstack = false;
	keep_timeline = false;
	lock_events = true;
	memory_events = true;
	sequence = 0;
	event_pos = 0;
	event_count = 0;
	overwritten = 0;
	dropped = 0;
	events.SetCount(TIMING_DEFAULT_EVENT_CAPACITY);
	contexts.Clear();
	contexts.Add(String());
	callstacks.Clear();
	callstacks.Add(String());
	s_timing_active.store(enabled, std::memory_order_release);
	s_timing_lock_events.store(lock_events, std::memory_order_release);
	s_timing_memory_events.store(memory_events, std::memory_order_release);
#endif
}

TimingStore::~TimingStore()
{
#ifdef flagTIMING
	s_timing_active.store(false, std::memory_order_release);
	s_timing_lock_events.store(false, std::memory_order_release);
	s_timing_memory_events.store(false, std::memory_order_release);
#endif
}

void TimingStore::Clear()
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	sequence = 0;
	event_pos = 0;
	event_count = 0;
	overwritten = 0;
	dropped = 0;
	for(int i = 0; i < events.GetCount(); ++i)
		events[i] = RawEvent();
	contexts.Clear();
	contexts.Add(String());
	callstacks.Clear();
	callstacks.Add(String());
#endif
}

void TimingStore::SetEnabled(bool b)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	enabled = b;
	s_timing_active.store(b, std::memory_order_release);
#else
	(void)b;
#endif
}

bool TimingStore::IsEnabled() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return enabled;
#else
	return false;
#endif
}

void TimingStore::SetCaptureCallstack(bool b)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	capture_callstack = b;
#else
	(void)b;
#endif
}

bool TimingStore::IsCaptureCallstack() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return capture_callstack;
#else
	return false;
#endif
}

void TimingStore::SetKeepTimeline(bool b)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	keep_timeline = b;
#else
	(void)b;
#endif
}

bool TimingStore::IsKeepTimeline() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return keep_timeline;
#else
	return false;
#endif
}

void TimingStore::SetEventCapacity(int capacity)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	if(capacity < 0)
		capacity = 0;
	Mutex::Lock __(mutex);
	if(capacity == events.GetCount())
		return;

	Vector<RawEvent> retained;
	CopyRetainedEvents(retained);
	int keep = min(retained.GetCount(), capacity);
	if(keep < retained.GetCount())
		overwritten += retained.GetCount() - keep;

	Vector<RawEvent> replacement;
	replacement.SetCount(capacity);
	for(int i = 0; i < keep; ++i)
		replacement[i] = retained[retained.GetCount() - keep + i];
	events = pick(replacement);
	event_count = keep;
	event_pos = capacity ? keep % capacity : 0;
#else
	(void)capacity;
#endif
}

int TimingStore::GetEventCapacity() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return events.GetCount();
#else
	return 0;
#endif
}

void TimingStore::SetMaxStorageBytes(int64 bytes)
{
#ifdef flagTIMING
	int64 event_bytes = (int64)sizeof(RawEvent);
	int64 capacity64 = bytes > 0 && event_bytes > 0 ? bytes / event_bytes : 0;
	if(capacity64 > INT_MAX)
		capacity64 = INT_MAX;
	SetEventCapacity((int)capacity64);
#else
	(void)bytes;
#endif
}

void TimingStore::SetLockEvents(bool b)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	lock_events = b;
	s_timing_lock_events.store(b, std::memory_order_release);
#else
	(void)b;
#endif
}

bool TimingStore::IsLockEvents() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return lock_events;
#else
	return false;
#endif
}

void TimingStore::SetMemoryEvents(bool b)
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	memory_events = b;
	s_timing_memory_events.store(b, std::memory_order_release);
#else
	(void)b;
#endif
}

bool TimingStore::IsMemoryEvents() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return memory_events;
#else
	return false;
#endif
}

void TimingStore::Record(const char *label, const String& context, dword elapsed, int nesting)
{
#ifdef flagTIMING
	Record(TIMING_EVENT_SCOPE, label, InternContext(context), elapsed, nesting);
#else
	(void)label; (void)context; (void)elapsed; (void)nesting;
#endif
}

void TimingStore::Record(byte category, const char *label, int context_id, dword elapsed, int nesting, int64 value)
{
#ifdef flagTIMING
	if(!label || !*label)
		return;
	TimingInternalGuard guard;

	bool enabled_now = false;
	bool capture_now = false;
	{
		Mutex::Lock __(mutex);
		enabled_now = enabled;
		capture_now = capture_callstack;
	}
	if(!enabled_now)
		return;

	TimingCallstack callstack;
	uint64 thread_id = GetTimingThreadId();
	int callstack_id = 0;
	if(capture_now) {
		callstack.Capture(2);
		callstack_id = InternCallstack(callstack.ToString());
	}

	Mutex::Lock __(mutex);
	if(!enabled)
		return;
	if(events.IsEmpty()) {
		dropped++;
		return;
	}
	RawEvent& ev = events[event_pos];
	if(event_count == events.GetCount())
		overwritten++;
	else
		event_count++;
	ev.label = label;
	ev.thread_id = thread_id;
	ev.sequence = ++sequence;
	ev.dropped_before = overwritten + dropped;
	ev.elapsed = elapsed;
	ev.value = value;
	ev.nesting = nesting;
	ev.context_id = context_id >= 0 && context_id < contexts.GetCount() ? context_id : 0;
	ev.callstack_id = callstack_id >= 0 && callstack_id < callstacks.GetCount() ? callstack_id : 0;
	ev.thread_name = t_timing_thread_name;
	ev.category = category;
	event_pos++;
	if(event_pos >= events.GetCount())
		event_pos = 0;
#else
	(void)category; (void)label; (void)context_id; (void)elapsed; (void)nesting; (void)value;
#endif
}

void TimingStore::Marker(const char *label, int context_id)
{
#ifdef flagTIMING
	Record(TIMING_EVENT_MARKER, label, context_id, 0, 0);
#else
	(void)label; (void)context_id;
#endif
}

int TimingStore::InternContext(const String& context)
{
#ifdef flagTIMING
	if(context.IsEmpty())
		return 0;
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	for(int i = 1; i < contexts.GetCount(); ++i)
		if(contexts[i] == context)
			return i;
	contexts.Add(context);
	return contexts.GetCount() - 1;
#else
	(void)context;
	return 0;
#endif
}

#ifdef flagTIMING
String TimingStore::GetContextText(int context_id) const
{
	if(context_id > 0 && context_id < contexts.GetCount())
		return contexts[context_id];
	return String();
}

String TimingStore::GetCallstackText(int callstack_id) const
{
	if(callstack_id > 0 && callstack_id < callstacks.GetCount())
		return callstacks[callstack_id];
	return String();
}

int TimingStore::InternCallstack(const String& callstack)
{
	if(callstack.IsEmpty())
		return 0;
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	for(int i = 1; i < callstacks.GetCount(); ++i)
		if(callstacks[i] == callstack)
			return i;
	callstacks.Add(callstack);
	return callstacks.GetCount() - 1;
}

int TimingStore::GetRetainedBegin() const
{
	int capacity = events.GetCount();
	if(!capacity)
		return 0;
	int begin = event_pos - event_count;
	if(begin < 0)
		begin += capacity;
	return begin;
}

void TimingStore::CopyRetainedEvents(Vector<RawEvent>& out) const
{
	out.Clear();
	out.Reserve(event_count);
	int capacity = events.GetCount();
	int begin = GetRetainedBegin();
	for(int i = 0; i < event_count; ++i)
		out.Add(events[(begin + i) % capacity]);
}

TimingSample TimingStore::MakeSample(const RawEvent& ev) const
{
	TimingSample sample;
	sample.key = TimingKey(ev.category, ev.label, GetContextText(ev.context_id), ev.thread_id, GetCallstackText(ev.callstack_id), ev.thread_name);
	sample.elapsed = ev.elapsed;
	sample.value = ev.value;
	sample.nesting = ev.nesting;
	sample.sequence = ev.sequence;
	sample.dropped_before = ev.dropped_before;
	return sample;
}

void TimingStore::AddRecord(VectorMap<TimingKey, TimingRecord>& records, const RawEvent& ev) const
{
	TimingKey key(ev.category, ev.label, GetContextText(ev.context_id), ev.thread_id, GetCallstackText(ev.callstack_id), ev.thread_name);
	TimingRecord& rec = records.GetAdd(key);
	if(rec.count == 0) {
		rec.key = key;
		rec.min_time = ev.elapsed;
		rec.max_time = ev.elapsed;
		rec.min_value = ev.value;
		rec.max_value = ev.value;
		rec.first_sequence = ev.sequence;
		rec.dropped_before = ev.dropped_before;
	}
	rec.count++;
	rec.total_time += ev.elapsed;
	rec.total_value += ev.value;
	if(ev.elapsed < rec.min_time)
		rec.min_time = ev.elapsed;
	if(ev.elapsed > rec.max_time)
		rec.max_time = ev.elapsed;
	if(ev.value < rec.min_value)
		rec.min_value = ev.value;
	if(ev.value > rec.max_value)
		rec.max_value = ev.value;
	if(ev.nesting > rec.max_nesting)
		rec.max_nesting = ev.nesting;
	rec.last_sequence = ev.sequence;
	if(ev.dropped_before > rec.dropped_before)
		rec.dropped_before = ev.dropped_before;
}
#endif

Vector<TimingRecord> TimingStore::GetRecords() const
{
	Vector<TimingRecord> out;
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	VectorMap<TimingKey, TimingRecord> records;
	records.Reserve(event_count);
	int capacity = events.GetCount();
	int begin = capacity ? event_pos - event_count : 0;
	if(begin < 0)
		begin += capacity;
	for(int i = 0; i < event_count; ++i)
		AddRecord(records, events[(begin + i) % capacity]);
	out.Reserve(records.GetCount());
	for(const auto& rec : records)
		out.Add(rec);
#endif
	return out;
}

Vector<TimingSample> TimingStore::GetTimeline() const
{
	Vector<TimingSample> out;
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	if(!keep_timeline)
		return out;
	out.Reserve(event_count);
	int capacity = events.GetCount();
	int begin = capacity ? event_pos - event_count : 0;
	if(begin < 0)
		begin += capacity;
	for(int i = 0; i < event_count; ++i)
		out.Add(MakeSample(events[(begin + i) % capacity]));
#endif
	return out;
}

String TimingStore::Dump() const
{
	String out;
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	VectorMap<TimingKey, TimingRecord> records;
	records.Reserve(event_count);
	int capacity = events.GetCount();
	int begin = capacity ? event_pos - event_count : 0;
	if(begin < 0)
		begin += capacity;
	for(int i = 0; i < event_count; ++i)
		AddRecord(records, events[(begin + i) % capacity]);

	out << "TimingStore records=" << records.GetCount()
	    << " events=" << event_count << "/" << events.GetCount()
	    << " sequence=" << sequence
	    << " overwritten=" << overwritten
	    << " dropped=" << dropped
	    << " estimated_event_bytes=" << (int64)sizeof(RawEvent)
	    << " enabled=" << enabled
	    << " callstack=" << capture_callstack
	    << " keep_timeline=" << keep_timeline
	    << " lock_events=" << lock_events
	    << " memory_events=" << memory_events
	    << '\n';
	if(event_count > 0) {
		int retained_begin = GetRetainedBegin();
		out << "Retained sequence range "
		    << events[retained_begin].sequence << ".."
		    << events[(retained_begin + event_count - 1) % events.GetCount()].sequence
		    << '\n';
	}
	if(overwritten || dropped)
		out << "Retention note: raw event storage is a bounded ring; overwritten context interval begin/end pairs can be retained without their matching endpoint.\n";
	for(const auto& rec : records)
		out << rec.ToString() << '\n';
	if(keep_timeline) {
		out << "Timeline:\n";
		for(int i = 0; i < event_count; ++i)
			out << MakeSample(events[(begin + i) % capacity]).ToString() << '\n';
	}
#endif
	return out;
}

TimingStorageStats TimingStore::GetStorageStats() const
{
	TimingStorageStats stats;
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	stats.event_capacity = events.GetCount();
	stats.event_count = event_count;
	stats.sequence = sequence;
	stats.overwritten = overwritten;
	stats.dropped = dropped;
	stats.estimated_event_bytes = (int64)sizeof(RawEvent);
	if(event_count > 0) {
		int begin = GetRetainedBegin();
		stats.retained_from_sequence = events[begin].sequence;
		stats.retained_to_sequence = events[(begin + event_count - 1) % events.GetCount()].sequence;
	}
#endif
	return stats;
}

ValueMap TimingStore::GetValueMap(bool include_timeline) const
{
	ValueMap out;
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	VectorMap<TimingKey, TimingRecord> records;
	records.Reserve(event_count);
	int capacity = events.GetCount();
	int begin = GetRetainedBegin();
	for(int i = 0; i < event_count; ++i)
		AddRecord(records, events[(begin + i) % capacity]);

	TimingStorageStats stats;
	stats.event_capacity = events.GetCount();
	stats.event_count = event_count;
	stats.sequence = sequence;
	stats.overwritten = overwritten;
	stats.dropped = dropped;
	stats.estimated_event_bytes = (int64)sizeof(RawEvent);
	if(event_count > 0) {
		stats.retained_from_sequence = events[begin].sequence;
		stats.retained_to_sequence = events[(begin + event_count - 1) % capacity].sequence;
	}

	ValueMap settings;
	settings.Add("enabled", enabled);
	settings.Add("capture_callstack", capture_callstack);
	settings.Add("keep_timeline", keep_timeline);
	settings.Add("lock_events", lock_events);
	settings.Add("memory_events", memory_events);

	ValueArray record_values;
	for(const auto& rec : records)
		record_values.Add(rec.ToValueMap());

	out.Add("stats", stats.ToValueMap());
	out.Add("settings", settings);
	out.Add("records", record_values);
	out.Add("timeline_included", include_timeline && keep_timeline);
	if(overwritten || dropped)
		out.Add("retention_note", "Raw event storage is a bounded ring; overwritten context interval begin/end pairs can be retained without their matching endpoint.");
	if(include_timeline && keep_timeline) {
		ValueArray timeline_values;
		for(int i = 0; i < event_count; ++i)
			timeline_values.Add(MakeSample(events[(begin + i) % capacity]).ToValueMap());
		out.Add("timeline", timeline_values);
	}
#else
	out.Add("stats", TimingStorageStats().ToValueMap());
	ValueMap settings;
	settings.Add("enabled", false);
	out.Add("settings", settings);
	out.Add("records", ValueArray());
	out.Add("timeline_included", false);
	(void)include_timeline;
#endif
	return out;
}

String TimingStore::GetJson(bool include_timeline, bool pretty) const
{
	return AsJSON(GetValueMap(include_timeline), pretty);
}

int TimingStore::GetRecordCount() const
{
#ifdef flagTIMING
	return GetRecords().GetCount();
#else
	return 0;
#endif
}

int TimingStore::GetTimelineCount() const
{
#ifdef flagTIMING
	TimingInternalGuard guard;
	Mutex::Lock __(mutex);
	return keep_timeline ? event_count : 0;
#else
	return 0;
#endif
}

TimingManager& TimingManager::Global()
{
#ifdef flagTIMING
	static TimingManager s;
	return s;
#else
	static TimingManager s;
	return s;
#endif
}

void TimingMemoryAllocHook(size_t size)
{
#ifdef flagTIMING
	if(TimingManager::IsMemoryEventCollectionActive())
		TimingManager::Global().RecordMemoryAlloc(size);
#else
	(void)size;
#endif
}

void TimingMemoryFreeHook(size_t size)
{
#ifdef flagTIMING
	if(TimingManager::IsMemoryEventCollectionActive())
		TimingManager::Global().RecordMemoryFree(size);
#else
	(void)size;
#endif
}

void TimingManager::Activate(bool b)
{
#ifdef flagTIMING
	store.SetEnabled(b);
#else
	(void)b;
#endif
}

bool TimingManager::IsActive() const
{
#ifdef flagTIMING
	return store.IsEnabled();
#else
	return false;
#endif
}

void TimingManager::SetCaptureCallstack(bool b)
{
#ifdef flagTIMING
	store.SetCaptureCallstack(b);
#else
	(void)b;
#endif
}

bool TimingManager::IsCaptureCallstack() const
{
#ifdef flagTIMING
	return store.IsCaptureCallstack();
#else
	return false;
#endif
}

void TimingManager::SetKeepTimeline(bool b)
{
#ifdef flagTIMING
	store.SetKeepTimeline(b);
#else
	(void)b;
#endif
}

bool TimingManager::IsKeepTimeline() const
{
#ifdef flagTIMING
	return store.IsKeepTimeline();
#else
	return false;
#endif
}

void TimingManager::SetEventCapacity(int capacity)
{
#ifdef flagTIMING
	store.SetEventCapacity(capacity);
#else
	(void)capacity;
#endif
}

int TimingManager::GetEventCapacity() const
{
#ifdef flagTIMING
	return store.GetEventCapacity();
#else
	return 0;
#endif
}

void TimingManager::SetMaxStorageBytes(int64 bytes)
{
#ifdef flagTIMING
	store.SetMaxStorageBytes(bytes);
#else
	(void)bytes;
#endif
}

void TimingManager::SetLockEvents(bool b)
{
#ifdef flagTIMING
	store.SetLockEvents(b);
#else
	(void)b;
#endif
}

bool TimingManager::IsLockEvents() const
{
#ifdef flagTIMING
	return store.IsLockEvents();
#else
	return false;
#endif
}

void TimingManager::SetMemoryEvents(bool b)
{
#ifdef flagTIMING
	store.SetMemoryEvents(b);
#else
	(void)b;
#endif
}

bool TimingManager::IsMemoryEvents() const
{
#ifdef flagTIMING
	return store.IsMemoryEvents();
#else
	return false;
#endif
}

void TimingManager::SetContext(const String& context)
{
#ifdef flagTIMING
	t_timing_context = context;
	t_timing_context_id = store.InternContext(context);
	store.Record(TIMING_EVENT_CONTEXT, "TimingContext", t_timing_context_id, 0, t_timing_nesting);
#else
	(void)context;
#endif
}

String TimingManager::GetContext() const
{
#ifdef flagTIMING
	return t_timing_context;
#else
	return String();
#endif
}

void TimingManager::SetThreadName(const char *name)
{
#ifdef flagTIMING
	t_timing_thread_name = name && *name ? name : NULL;
#else
	(void)name;
#endif
}

String TimingManager::GetThreadName() const
{
#ifdef flagTIMING
	return t_timing_thread_name ? String(t_timing_thread_name) : String();
#else
	return String();
#endif
}

void TimingManager::BeginContext(const char *label, const String& metadata)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !label || !*label)
		return;
	store.Record(TIMING_EVENT_CONTEXT, label, store.InternContext(metadata), 0, t_timing_nesting, 1);
#else
	(void)label; (void)metadata;
#endif
}

void TimingManager::EndContext(const char *label, const String& metadata)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !label || !*label)
		return;
	store.Record(TIMING_EVENT_CONTEXT, label, store.InternContext(metadata), 0, t_timing_nesting, -1);
#else
	(void)label; (void)metadata;
#endif
}

void TimingManager::SetMetadata(const char *label, const String& metadata)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !label || !*label)
		return;
	store.Record(TIMING_EVENT_METADATA_UPDATE, label, store.InternContext(metadata), 0, t_timing_nesting, 0);
#else
	(void)label; (void)metadata;
#endif
}

void TimingManager::Marker(const char *label)
{
#ifdef flagTIMING
	store.Marker(label, t_timing_context_id);
#else
	(void)label;
#endif
}

void TimingManager::Clear()
{
#ifdef flagTIMING
	store.Clear();
	t_timing_context_id = 0;
#endif
}

void TimingManager::Record(const char *label, dword elapsed, int nesting)
{
#ifdef flagTIMING
	store.Record(TIMING_EVENT_SCOPE, label, t_timing_context_id, elapsed, nesting);
#else
	(void)label; (void)elapsed; (void)nesting;
#endif
}

void TimingManager::RecordLockWait(const char *label, dword elapsed)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsLockEvents())
		return;
	store.Record(TIMING_EVENT_MUTEX_WAIT, label, t_timing_context_id, elapsed, t_timing_nesting);
#else
	(void)label; (void)elapsed;
#endif
}

void TimingManager::RecordLockHold(const char *label, dword elapsed)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsLockEvents())
		return;
	store.Record(TIMING_EVENT_MUTEX_HOLD, label, t_timing_context_id, elapsed, t_timing_nesting);
#else
	(void)label; (void)elapsed;
#endif
}

void TimingManager::RecordSemaphoreWait(const char *label, dword elapsed)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsLockEvents())
		return;
	store.Record(TIMING_EVENT_SEMAPHORE_WAIT, label, t_timing_context_id, elapsed, t_timing_nesting);
#else
	(void)label; (void)elapsed;
#endif
}

void TimingManager::RecordSpinWait(const char *label, dword elapsed)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsLockEvents())
		return;
	store.Record(TIMING_EVENT_SPIN_WAIT, label, t_timing_context_id, elapsed, t_timing_nesting);
#else
	(void)label; (void)elapsed;
#endif
}

void TimingManager::RecordMemoryAlloc(size_t size)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsMemoryEvents())
		return;
	const char *label = t_timing_scope_label ? t_timing_scope_label : "MemoryAlloc";
	store.Record(TIMING_EVENT_MEMORY_ALLOC, label, t_timing_context_id, 0, t_timing_nesting, (int64)size);
#else
	(void)size;
#endif
}

void TimingManager::RecordMemoryFree(size_t size)
{
#ifdef flagTIMING
	if(t_timing_suppressed || !store.IsEnabled() || !store.IsMemoryEvents())
		return;
	const char *label = t_timing_scope_label ? t_timing_scope_label : "MemoryFree";
	store.Record(TIMING_EVENT_MEMORY_FREE, label, t_timing_context_id, 0, t_timing_nesting, -(int64)size);
#else
	(void)size;
#endif
}

bool TimingManager::IsProfilingSuppressed()
{
#ifdef flagTIMING
	return t_timing_suppressed;
#else
	return true;
#endif
}

bool TimingManager::IsLockEventCollectionActive()
{
#ifdef flagTIMING
	return !t_timing_suppressed &&
	       s_timing_active.load(std::memory_order_acquire) &&
	       s_timing_lock_events.load(std::memory_order_acquire);
#else
	return false;
#endif
}

bool TimingManager::IsMemoryEventCollectionActive()
{
#ifdef flagTIMING
	return !t_timing_suppressed &&
	       s_timing_active.load(std::memory_order_acquire) &&
	       s_timing_memory_events.load(std::memory_order_acquire);
#else
	return false;
#endif
}

Vector<TimingRecord> TimingManager::GetRecords() const
{
#ifdef flagTIMING
	return store.GetRecords();
#else
	return Vector<TimingRecord>();
#endif
}

Vector<TimingSample> TimingManager::GetTimeline() const
{
#ifdef flagTIMING
	return store.GetTimeline();
#else
	return Vector<TimingSample>();
#endif
}

TimingStorageStats TimingManager::GetStorageStats() const
{
#ifdef flagTIMING
	return store.GetStorageStats();
#else
	return TimingStorageStats();
#endif
}

ValueMap TimingManager::GetValueMap(bool include_timeline) const
{
#ifdef flagTIMING
	return store.GetValueMap(include_timeline);
#else
	ValueMap out;
	out.Add("stats", TimingStorageStats().ToValueMap());
	ValueMap settings;
	settings.Add("enabled", false);
	out.Add("settings", settings);
	out.Add("records", ValueArray());
	out.Add("timeline_included", false);
	(void)include_timeline;
	return out;
#endif
}

String TimingManager::GetJson(bool include_timeline, bool pretty) const
{
	return AsJSON(GetValueMap(include_timeline), pretty);
}

String TimingManager::Dump() const
{
#ifdef flagTIMING
	return store.Dump();
#else
	return String();
#endif
}

TimingScope::TimingScope(const char *label)
	: label(label)
{
#ifdef flagTIMING
	active = TimingManager::Global().IsActive() && label && *label;
	if(active) {
		start_time = tmGetTime();
		prev_label = t_timing_scope_label;
		t_timing_scope_label = label;
		++t_timing_nesting;
	}
#else
	(void)label;
#endif
}

TimingScope::~TimingScope()
{
#ifdef flagTIMING
	if(!active)
		return;
	--t_timing_nesting;
	TimingManager::Global().Record(label, tmGetTime() - start_time, t_timing_nesting);
	t_timing_scope_label = prev_label;
#endif
}

TimingContextScope::TimingContextScope(const String& context)
{
#ifdef flagTIMING
	prev = TimingManager::Global().GetContext();
	TimingManager::Global().SetContext(context);
	active = true;
#else
	(void)context;
#endif
}

TimingContextScope::~TimingContextScope()
{
#ifdef flagTIMING
	if(active)
		TimingManager::Global().SetContext(prev);
#endif
}

TimingContextInterval::TimingContextInterval(const char *label, const String& metadata)
	: label(label), metadata(metadata)
{
#ifdef flagTIMING
	active = TimingManager::Global().IsActive() && label && *label;
	if(active)
		TimingManager::Global().BeginContext(label, metadata);
#else
	(void)label;
#endif
}

TimingContextInterval::~TimingContextInterval()
{
#ifdef flagTIMING
	if(active)
		TimingManager::Global().EndContext(label, metadata);
#endif
}

} // namespace Upp
