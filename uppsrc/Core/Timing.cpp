#include "Core.h"

#ifdef PLATFORM_POSIX
#include <execinfo.h>
#include <vector>
#endif

namespace Upp {

namespace {

#ifdef flagTIMING
	thread_local String t_timing_context;
	thread_local int    t_timing_nesting = 0;
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

} // namespace

hash_t TimingKey::GetHashValue() const
{
	return CombineHash(label, context, callstack, thread_id);
}

TimingKey::TimingKey()
	: thread_id(0)
{}

TimingKey::TimingKey(const String& label, const String& context, uint64 thread_id, const String& callstack)
	: label(label), context(context), callstack(callstack), thread_id(thread_id)
{}

bool TimingKey::operator<(const TimingKey& b) const
{
	if(thread_id != b.thread_id)
		return thread_id < b.thread_id;
	if(context != b.context)
		return context < b.context;
	if(label != b.label)
		return label < b.label;
	return callstack < b.callstack;
}

bool TimingKey::operator==(const TimingKey& b) const
{
	return thread_id == b.thread_id && context == b.context && label == b.label && callstack == b.callstack;
}

String TimingKey::ToString() const
{
	String s;
	s << "thread=" << Format64Hex(thread_id);
	if(!context.IsEmpty())
		s << " context=" << context;
	if(!label.IsEmpty())
		s << " label=" << label;
	if(!callstack.IsEmpty())
		s << " callstack=" << callstack;
	return s;
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

String TimingSample::ToString() const
{
	String s;
	s << key.ToString()
	  << " elapsed=" << AsString(elapsed)
	  << " nesting=" << AsString(nesting)
	  << " seq=" << AsString(sequence);
	return s;
}

String TimingRecord::ToString() const
{
	String s;
	s << key.ToString()
	  << " count=" << AsString(count)
	  << " total=" << AsString(total_time)
	  << " min=" << AsString(min_time)
	  << " max=" << AsString(max_time)
	  << " nesting=" << AsString(max_nesting)
	  << " first=" << AsString(first_sequence)
	  << " last=" << AsString(last_sequence);
	return s;
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
	sequence = 0;
#endif
}

void TimingStore::Clear()
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	records.Clear();
	timeline.Clear();
	sequence = 0;
#endif
}

void TimingStore::SetEnabled(bool b)
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	enabled = b;
#else
	(void)b;
#endif
}

bool TimingStore::IsEnabled() const
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	return enabled;
#else
	return false;
#endif
}

void TimingStore::SetCaptureCallstack(bool b)
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	capture_callstack = b;
#else
	(void)b;
#endif
}

bool TimingStore::IsCaptureCallstack() const
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	return capture_callstack;
#else
	return false;
#endif
}

void TimingStore::SetKeepTimeline(bool b)
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	keep_timeline = b;
#else
	(void)b;
#endif
}

bool TimingStore::IsKeepTimeline() const
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	return keep_timeline;
#else
	return false;
#endif
}

void TimingStore::Record(const char *label, const String& context, dword elapsed, int nesting)
{
#ifdef flagTIMING
	if(!label || !*label)
		return;

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
	String callstack_text;
	if(capture_now) {
		callstack.Capture(2);
		callstack_text = callstack.ToString();
	}

	TimingKey key(label, context, thread_id, callstack_text);
	TimingSample sample;
	sample.key = key;
	sample.elapsed = elapsed;
	sample.nesting = nesting;

	Mutex::Lock __(mutex);
	if(!enabled)
		return;
	sample.sequence = ++sequence;
	TimingRecord& rec = records.GetAdd(key);
	if(rec.count == 0) {
		rec.key = key;
		rec.min_time = elapsed;
		rec.max_time = elapsed;
		rec.first_sequence = sample.sequence;
	}
	rec.count++;
	rec.total_time += elapsed;
	if(elapsed < rec.min_time)
		rec.min_time = elapsed;
	if(elapsed > rec.max_time)
		rec.max_time = elapsed;
	if(nesting > rec.max_nesting)
		rec.max_nesting = nesting;
	rec.last_sequence = sample.sequence;
	if(keep_timeline)
		timeline.Add(sample);
#else
	(void)label; (void)context; (void)elapsed; (void)nesting;
#endif
}

Vector<TimingRecord> TimingStore::GetRecords() const
{
	Vector<TimingRecord> out;
#ifdef flagTIMING
	Mutex::Lock __(mutex);
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
	Mutex::Lock __(mutex);
	out.Append(timeline);
#endif
	return out;
}

String TimingStore::Dump() const
{
	String out;
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	out << "TimingStore records=" << records.GetCount()
	    << " timeline=" << timeline.GetCount()
	    << " enabled=" << enabled
	    << " callstack=" << capture_callstack
	    << " keep_timeline=" << keep_timeline
	    << '\n';
	for(const auto& rec : records)
		out << rec.ToString() << '\n';
	if(keep_timeline) {
		out << "Timeline:\n";
		for(int i = 0; i < timeline.GetCount(); ++i)
			out << timeline[i].ToString() << '\n';
	}
#endif
	return out;
}

int TimingStore::GetRecordCount() const
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	return records.GetCount();
#else
	return 0;
#endif
}

int TimingStore::GetTimelineCount() const
{
#ifdef flagTIMING
	Mutex::Lock __(mutex);
	return timeline.GetCount();
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

void TimingManager::SetContext(const String& context)
{
#ifdef flagTIMING
	t_timing_context = context;
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

void TimingManager::Clear()
{
#ifdef flagTIMING
	store.Clear();
#endif
}

void TimingManager::Record(const char *label, dword elapsed, int nesting)
{
#ifdef flagTIMING
	store.Record(label, GetContext(), elapsed, nesting);
#else
	(void)label; (void)elapsed; (void)nesting;
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

} // namespace Upp
