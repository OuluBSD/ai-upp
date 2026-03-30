#ifndef _MluiViewer_MluiViewer_h_
#define _MluiViewer_MluiViewer_h_

#include <CtrlLib/CtrlLib.h>

class MluiLayoutView : public Upp::Ctrl {
public:
	struct Item : Upp::Moveable<Item> {
		Upp::String key;
		Upp::String label;
		Upp::Rect   rect = Upp::Null;
		int         semantic_depth = 0;
		int         semantic_type_rank = 10;
		bool        clickable = false;
	};

	void SetItems(const Upp::Vector<Item>& items);
	void SetSelectedKey(const Upp::String& key);
	const Upp::String& GetSelectedKey() const { return selected_key; }

	Upp::Event<Upp::String> WhenSelectKey;

private:
	Upp::Vector<Item> items;
	Upp::Vector<Upp::Rect> screen_rects;
	Upp::String selected_key;

	Upp::Color GetFillColor(const Upp::String& key) const;
	Upp::Rect  MapRect(const Upp::Rect& world, const Upp::Rect& bounds, double scale, Upp::Point off) const;

protected:
	void Paint(Upp::Draw& w) override;
	void LeftDown(Upp::Point p, Upp::dword flags) override;
};

class MluiViewerWindow : public Upp::TopWindow {
public:
	typedef MluiViewerWindow CLASSNAME;

	MluiViewerWindow();
	void SetAddress(const Upp::String& addr);
	void EnableAutoRefresh(int period_ms);

private:
	Upp::Label     address_lbl;
	Upp::EditString address;
	Upp::Button    refresh;
	Upp::Button    click_selected;
	Upp::Label     input_lbl;
	Upp::EditString input;
	Upp::Button    send_input;

	Upp::TreeArrayCtrl elements;
	Upp::TabCtrl   right_tabs;
	Upp::ParentCtrl right_layout_tab;
	Upp::ParentCtrl right_json_tab;
	MluiLayoutView layout_view;
	Upp::DocEdit   raw_json;
	Upp::Splitter  split;
	Upp::TimeCallback auto_refresh_cb;
	bool auto_refresh_enabled = false;
	int auto_refresh_ms = 1000;

	int request_id = 1;

	void LayoutControls();
	void InitTree();
	int  FindChildByKey(const Upp::TreeArrayCtrl& tree, int parent_id, const Upp::String& key) const;
	int  EnsureTreeNode(Upp::TreeArrayCtrl& tree, int parent_id, const Upp::String& key, const Upp::String& label);
	void CollectStaleNodes(const Upp::TreeArrayCtrl& tree, int id, const Upp::Index<Upp::String>& alive_keys, Upp::Vector<int>& out_remove) const;
	Upp::Vector<Upp::String> ParseSemanticPath(const Upp::String& semantic_path) const;
	Upp::String BuildLeafKey(const Upp::String& semantic_path, const Upp::String& path) const;
	bool ExtractLeafKey(const Upp::String& key, Upp::String& out_semantic, Upp::String& out_path) const;
	bool ResolveClickTarget(const Upp::TreeArrayCtrl& tree, int id, Upp::String& out_target) const;
	int  FindNodeByKey(const Upp::TreeArrayCtrl& tree, int id, const Upp::String& key) const;
	int  FindNodeByKey(const Upp::TreeArrayCtrl& tree, const Upp::String& key) const;
	int  FindBestActionableDescendant(const Upp::TreeArrayCtrl& tree, int id) const;

	struct SnapshotEntry : Upp::Moveable<SnapshotEntry> {
		Upp::String semantic;
		Upp::String path;
		Upp::String type;
		Upp::String text;
		Upp::String visible_text;
		bool        is_visible = true;
		Upp::String rect_text;
		double ratio = 0.0;
		Upp::Rect rect = Upp::Null;
	};

	void ParseSnapshotEntries(const Upp::ValueArray& elements_json, Upp::Vector<SnapshotEntry>& out);
	void UpdateDataTreeFromEntries(const Upp::Vector<SnapshotEntry>& entries);
	void UpdateLayoutViewFromEntries(const Upp::Vector<SnapshotEntry>& entries);

	struct SnapshotJobResult {
		bool show_errors = false;
		bool transport_ok = false;
		bool rpc_ok = false;
		Upp::String error;
		Upp::Value result;
	};

	void RequestSnapshot(bool show_errors);
	void OnSnapshotResult(SnapshotJobResult* result);
	void RefreshSnapshot();
	void RefreshSnapshotSilent();
	void AutoRefreshTick();
	void ClickSelected();
	void SendInput();
	void OnElementsCursor();
	void OnLayoutViewSelect(Upp::String key);

	bool snapshot_request_running = false;
	bool snapshot_request_pending = false;
	bool snapshot_pending_show_errors = false;
};

#endif
