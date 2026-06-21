#ifndef _CtrlCore_Timing_h_
#define _CtrlCore_Timing_h_

class TimingView : public Ctrl {
public:
	typedef TimingView CLASSNAME;

	TimingView();

	void   SetTimelineMode(bool b);
	bool   IsTimelineMode() const                 { return timeline_mode; }

	void   SetContextIntervalMode(bool b);
	bool   IsContextIntervalMode() const          { return context_interval_mode; }

	void   SetContextFilter(const String& s);
	const String& GetContextFilter() const        { return context_filter; }

	void   SetThreadFilter(uint64 id);
	uint64 GetThreadFilter() const                { return thread_filter; }

	void   SetCategoryMask(uint64 mask);
	uint64 GetCategoryMask() const                { return category_mask; }

	void   SetTopRow(int row);
	int    GetTopRow() const                      { return top_row; }

	void   RefreshData();
	void   SelectRow(int row);
	int    GetSelectedRow() const                 { return selected_row; }
	String GetSelectedText() const;

	virtual Vector<TimingRecord> GetRecords() const;
	virtual Vector<TimingSample> GetTimeline() const;
	virtual TimingStorageStats GetStorageStats() const;

	Callback1<int> WhenSelect;

protected:
	void   Paint(Draw& w) override;
	bool   Key(dword key, int count) override;
	void   MouseWheel(Point p, int zdelta, dword keyflags) override;
	void   LeftDown(Point p, dword keyflags) override;
	void   GotFocus() override;
	void   LostFocus() override;

private:
	struct ContextItem : Moveable<ContextItem> {
		TimingSample begin;
		TimingSample end;
		TimingSample point;
		TimingRecord summary;
		bool         has_begin = false;
		bool         has_end = false;
		bool         has_point = false;
		bool         has_summary = false;
		bool         metadata = false;
		int          lane = 0;
	};

	Vector<TimingRecord> records;
	Vector<TimingSample>  timeline;
	Vector<ContextItem>   context_items;
	String                context_filter;
	uint64                thread_filter = 0;
	uint64                category_mask = (uint64)-1;
	TimingStorageStats    stats;
	bool                  timeline_mode = false;
	bool                  context_interval_mode = false;
	bool                  dirty = true;
	int                   top_row = 0;
	int                   line_height = 18;
	int                   selected_row = -1;

	bool   MatchKey(const TimingKey& key) const;
	bool   MatchCategory(byte category) const;
	void   CollectData();
	void   BuildContextItems();
	void   Normalize();
	void   EnsureSelectionVisible();
	int    RowCount() const;
	String RowText(int row) const;
	String ContextItemText(const ContextItem& item, int row) const;
	int    RowAt(Point p) const;
	Color  RowColor(byte category, int row) const;
	byte   RowCategory(int row) const;
	void   PaintContextItem(Draw& w, const ContextItem& item, int y, int width) const;
};

#endif
