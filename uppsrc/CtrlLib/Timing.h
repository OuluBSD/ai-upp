#ifndef _CtrlLib_Timing_h_
#define _CtrlLib_Timing_h_

class TimingWidget : public Ctrl {
public:
	typedef TimingWidget CLASSNAME;

	TimingWidget();

	void   RefreshData();

	void   SetTimelineMode(bool b);
	bool   IsTimelineMode() const                { return timeline.Get(); }

	void   SetContextFilter(const String& s);
	String GetContextFilter() const;

	void   SetThreadFilter(uint64 id);
	uint64 GetThreadFilter() const;

	void   SetActive(bool b);
	bool   IsActive() const                      { return active.Get(); }

	void   SetCaptureCallstack(bool b);
	bool   IsCaptureCallstack() const            { return capture.Get(); }

	void   SetKeepTimeline(bool b);
	bool   IsKeepTimeline() const                { return keep_timeline.Get(); }

	TimingView&       GetView()                  { return view; }
	const TimingView& GetView() const            { return view; }

protected:
	void   Layout() override;

private:
	Label      context_lbl;
	EditString  context;
	Label      thread_lbl;
	EditString  thread;
	Option      timeline;
	Option      active;
	Option      capture;
	Option      keep_timeline;
	Button      refresh;
	Button      clear;
	TimingView  view;
	DocEdit     details;

	void   ApplyFilters();
	void   SyncFromManager();
	void   ClearData();
	void   UpdateDetails();
	void   OnViewSelect(int row);
	bool   GetThreadValue(uint64& id) const;

	uint64 thread_filter = 0;
};

#endif
