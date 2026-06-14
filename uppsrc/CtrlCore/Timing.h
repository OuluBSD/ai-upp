#ifndef _CtrlCore_Timing_h_
#define _CtrlCore_Timing_h_

class TimingView : public Ctrl {
public:
	typedef TimingView CLASSNAME;

	TimingView();

	void   SetTimelineMode(bool b);
	bool   IsTimelineMode() const                 { return timeline_mode; }

	void   SetContextFilter(const String& s);
	const String& GetContextFilter() const        { return context_filter; }

	void   SetThreadFilter(uint64 id);
	uint64 GetThreadFilter() const                { return thread_filter; }

	void   SetTopRow(int row);
	int    GetTopRow() const                      { return top_row; }

	void   RefreshData();
	void   SelectRow(int row);
	int    GetSelectedRow() const                 { return selected_row; }
	String GetSelectedText() const;

	virtual Vector<TimingRecord> GetRecords() const;
	virtual Vector<TimingSample> GetTimeline() const;

	Callback1<int> WhenSelect;

protected:
	void   Paint(Draw& w) override;
	bool   Key(dword key, int count) override;
	void   MouseWheel(Point p, int zdelta, dword keyflags) override;
	void   LeftDown(Point p, dword keyflags) override;
	void   GotFocus() override;
	void   LostFocus() override;

private:
	Vector<TimingRecord> records;
	Vector<TimingSample>  timeline;
	String                context_filter;
	uint64                thread_filter = 0;
	bool                  timeline_mode = false;
	bool                  dirty = true;
	int                   top_row = 0;
	int                   line_height = 18;
	int                   selected_row = -1;

	bool   MatchKey(const TimingKey& key) const;
	void   CollectData();
	void   Normalize();
	void   EnsureSelectionVisible();
	int    RowCount() const;
	String RowText(int row) const;
	int    RowAt(Point p) const;
};

#endif
