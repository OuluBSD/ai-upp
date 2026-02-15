#include "ModelerApp.h"
#include <Core/XML.h>

NAMESPACE_UPP

namespace {

class RibbonForm : public ParentCtrl {
public:
	struct Row : Moveable<Row> {
		int label_idx = -1;
		int ctrl_idx = -1;
		int height = 0;
	};

	Array<Label> labels;
	Array<Ctrl> ctrls;
	Vector<Row> rows;
	int row_cy = 24;
	int gap = 4;
	int label_cx = 120;

	void Clear()
	{
		labels.Clear();
		ctrls.Clear();
		rows.Clear();
	}

	void SetMetrics(int _row_cy, int _gap, int _label_cx)
	{
		row_cy = _row_cy;
		gap = _gap;
		label_cx = _label_cx;
		RefreshParentLayout();
	}

	void AddLabelOnly(const String& text)
	{
		Row& r = rows.Add();
		Label& l = labels.Add();
		l.SetText(text);
		Add(l);
		r.label_idx = labels.GetCount() - 1;
		r.ctrl_idx = -1;
		r.height = row_cy;
	}

	template <class T>
	T& AddRowCtrl(const String& label)
	{
		Row& r = rows.Add();
		if (!label.IsEmpty()) {
			Label& l = labels.Add();
			l.SetText(label);
			Add(l);
			r.label_idx = labels.GetCount() - 1;
		}
		T& c = static_cast<T&>(ctrls.Create<T>());
		Add(c);
		r.ctrl_idx = ctrls.GetCount() - 1;
		r.height = row_cy;
		return c;
	}

	Size GetMinSize() const override
	{
		int cy = 0;
		int max_cx = 0;
		for (const Row& r : rows) {
			cy += (r.height ? r.height : row_cy);
			if (r.ctrl_idx >= 0 && r.ctrl_idx < ctrls.GetCount()) {
				max_cx = max(max_cx, ctrls[r.ctrl_idx].GetMinSize().cx);
			}
		}
		int cx = label_cx + gap + max_cx;
		return Size(cx, cy);
	}

	void Layout() override
	{
		int y = 0;
		int cx = GetSize().cx;
		int value_cx = max(0, cx - label_cx - gap);
		for (const Row& r : rows) {
			int h = r.height ? r.height : row_cy;
			if (r.label_idx >= 0 && r.label_idx < labels.GetCount()) {
				labels[r.label_idx].SetRect(0, y, label_cx, h);
			}
			if (r.ctrl_idx >= 0 && r.ctrl_idx < ctrls.GetCount()) {
				ctrls[r.ctrl_idx].SetRect(label_cx + gap, y, value_cx, h);
			}
			y += h;
		}
	}
};

static Image RibbonIconForId(const String& id, const String& sem, const String& icon)
{
	String sid = ToLower(id);
	String ssem = ToLower(sem);
	String sicon = ToLower(icon);
	if (sid == "new_file" || ssem.Find("new") >= 0)
		return CtrlImg::new_doc();
	if (sid == "open_file" || ssem.Find("open") >= 0)
		return CtrlImg::open();
	if (sid == "save_file" || ssem.Find("save") >= 0)
		return CtrlImg::save();
	if (sid == "undo" || ssem.Find("undo") >= 0)
		return CtrlImg::undo();
	if (sid == "redo" || ssem.Find("redo") >= 0)
		return CtrlImg::redo();
	if (ssem.Find("camera") >= 0)
		return ImagesImg::Camera();
	if (ssem.Find("cube") >= 0 || ssem.Find("sphere") >= 0 || ssem.Find("cylinder") >= 0 ||
	    ssem.Find("cone") >= 0 || ssem.Find("plane") >= 0 || ssem.Find("mesh") >= 0)
		return ImagesImg::Model();
	if (ssem.Find("light") >= 0 || ssem.Find("sun") >= 0)
		return ImagesImg::Object();
	if (ssem.Find("play") >= 0)
		return ImagesImg::Play();
	if (sicon == "plus")
		return CtrlImg::plus();
	if (sicon == "minus")
		return CtrlImg::minus();
	return CtrlImg::File();
}

static String NodeAttr(const XmlNode& n, const char* key)
{
	String v = n.Attr(key);
	return v;
}

} // namespace

RibbonCtrl::RibbonCtrl()
{
	Add(bar.SizePos());
	const RibbonStyle& s = RibbonBar::StyleDefault();
	int h = s.full_button.cy + s.label_gap + s.full_font.GetHeight() + DPI(32);
	Height(h);
	bar.WhenTabMenu = [this](Bar& bar) {
		bar.Add(t_("Show Tabs Only"), [this] { this->bar.SetDisplayMode(RibbonBar::RIBBON_TABS); })
			.Check(this->bar.GetDisplayMode() == RibbonBar::RIBBON_TABS);
		bar.Add(t_("Always Show Ribbon"), [this] { this->bar.SetDisplayMode(RibbonBar::RIBBON_ALWAYS); })
			.Check(this->bar.GetDisplayMode() == RibbonBar::RIBBON_ALWAYS);
		bar.Add(t_("Auto-hide Ribbon"), [this] { this->bar.SetDisplayMode(RibbonBar::RIBBON_AUTOHIDE); })
			.Check(this->bar.GetDisplayMode() == RibbonBar::RIBBON_AUTOHIDE);
		bar.Separator();
		bar.Add(t_("QAT Top"), [this] { SetQuickAccessPos(RibbonBar::QAT_TOP); })
			.Check(GetQuickAccessPos() == RibbonBar::QAT_TOP);
		bar.Add(t_("QAT Bottom"), [this] { SetQuickAccessPos(RibbonBar::QAT_BOTTOM); })
			.Check(GetQuickAccessPos() == RibbonBar::QAT_BOTTOM);
	};
}

void RibbonCtrl::Init(Edit3D* o)
{
	owner = o;
	SetupQuickAccess();
}

void RibbonCtrl::Clear()
{
	bar.ClearTabs();
	control_by_id.Clear();
	items.Clear();
	owned_ctrls.Clear();
	loaded = false;
}

bool RibbonCtrl::LoadSpec(const String& path)
{
	Clear();
	spec_path = path;
	String xml = LoadFile(path);
	if (xml.IsEmpty())
		return false;
	return LoadSpecXml(xml);
}

bool RibbonCtrl::LoadSpecXml(const String& xml)
{
	Clear();
	XmlNode root = ParseXML(xml);
	const XmlNode* ribbon = nullptr;
	if (root.IsTag("coppercube_ribbon_spec")) {
		for (int i = 0; i < root.GetCount(); i++) {
			const XmlNode& n = root.Node(i);
			if (n.IsTag("ribbon")) {
				ribbon = &n;
				break;
			}
		}
	}
	else if (root.IsTag("ribbon")) {
		ribbon = &root;
	}
	if (!ribbon)
		return false;
	BuildFromSpec(*ribbon);
	AddContextTabs();
	loaded = true;
	return true;
}

void RibbonCtrl::BuildFromSpec(const XmlNode& root)
{
	bar.ClearTabs();
	control_by_id.Clear();
	items.Clear();
	owned_ctrls.Clear();
	for (int i = 0; i < root.GetCount(); i++) {
		const XmlNode& tab = root.Node(i);
		if (!tab.IsTag("tab"))
			continue;
		String tab_id = NodeAttr(tab, "id");
		String tab_title = NodeAttr(tab, "title");
		if (tab_title.IsEmpty())
			tab_title = tab_id;
		RibbonPage& page = bar.AddTab(tab_title);

		for (int j = 0; j < tab.GetCount(); j++) {
			const XmlNode& gnode = tab.Node(j);
			if (!gnode.IsTag("group"))
				continue;
			String group_id = NodeAttr(gnode, "id");
			String group_title = NodeAttr(gnode, "title");
			if (group_title.IsEmpty())
				group_title = group_id;
			bool has_form = false;
			bool use_list = false;
			for (int k = 0; k < gnode.GetCount(); k++) {
				const XmlNode& cn = gnode.Node(k);
				if (!cn.IsTag())
					continue;
				String tag = ToLower(cn.GetTag());
				if (tag == "label" || tag == "dropdown" || tag == "edit_int" || tag == "edit_double" || tag == "checkbox")
					has_form = true;
				if (tag == "button" && ToLower(NodeAttr(cn, "style")) == "text")
					use_list = true;
			}
			RibbonGroup& group = page.AddGroup(group_title);

			if (has_form) {
				One<Ctrl>& slot = owned_ctrls.Add();
				slot.Create<RibbonForm>();
				RibbonForm& form = static_cast<RibbonForm&>(*slot);
				form.SetMetrics(DPI(24), DPI(6), DPI(120));
				String pending_label;
				String pending_for;
				for (int k = 0; k < gnode.GetCount(); k++) {
					const XmlNode& cn = gnode.Node(k);
					if (!cn.IsTag())
						continue;
					String tag = ToLower(cn.GetTag());
					if (tag == "label") {
						pending_label = NodeAttr(cn, "text");
						pending_for = NodeAttr(cn, "for");
						continue;
					}
					if (tag == "button") {
					String id = NodeAttr(cn, "id");
					String text = NodeAttr(cn, "text");
					Button& b = form.AddRowCtrl<Button>(String());
					b.SetLabel(text);
					b.WhenAction = [this, id] { OnAction(id); };
					if (!id.IsEmpty())
						control_by_id.GetAdd(id, &b);
					continue;
					}
					if (tag == "dropdown") {
						String id = NodeAttr(cn, "id");
						String label = pending_label;
						if (!pending_for.IsEmpty() && pending_for != id)
							label.Clear();
					DropList& dl = form.AddRowCtrl<DropList>(label);
					for (int m = 0; m < cn.GetCount(); m++) {
						const XmlNode& item = cn.Node(m);
						if (item.IsTag("item")) {
							String key = item.Attr("id");
							String text = item.Attr("text");
							if (key.IsEmpty())
								key = text;
							dl.Add(key, text);
						}
					}
					if (dl.GetCount() > 0)
						dl.SetIndex(0);
					dl.WhenAction = [this, id] { OnAction(id); };
					if (!id.IsEmpty())
						control_by_id.GetAdd(id, &dl);
					pending_label.Clear();
					pending_for.Clear();
					continue;
					}
					if (tag == "edit_int") {
						String id = NodeAttr(cn, "id");
						String label = pending_label;
						if (!pending_for.IsEmpty() && pending_for != id)
							label.Clear();
						EditIntSpin& ed = form.AddRowCtrl<EditIntSpin>(label);
					String def = NodeAttr(cn, "default");
					if (!def.IsEmpty())
						ed.SetData(StrInt(def));
					if (!id.IsEmpty())
						control_by_id.GetAdd(id, &ed);
					pending_label.Clear();
					pending_for.Clear();
					continue;
					}
					if (tag == "edit_double") {
						String id = NodeAttr(cn, "id");
						String label = pending_label;
						if (!pending_for.IsEmpty() && pending_for != id)
							label.Clear();
						EditDoubleSpin& ed = form.AddRowCtrl<EditDoubleSpin>(label);
					String def = NodeAttr(cn, "default");
					if (!def.IsEmpty())
						ed.SetData(StrDbl(def));
					if (!id.IsEmpty())
						control_by_id.GetAdd(id, &ed);
					pending_label.Clear();
					pending_for.Clear();
					continue;
					}
					if (tag == "checkbox") {
						String id = NodeAttr(cn, "id");
						String label = pending_label;
						if (label.IsEmpty())
							label = id;
						Option& opt = form.AddRowCtrl<Option>(label);
					String def = ToLower(NodeAttr(cn, "default"));
					if (def == "true" || def == "1")
						opt = true;
					if (!id.IsEmpty())
						control_by_id.GetAdd(id, &opt);
					pending_label.Clear();
					pending_for.Clear();
					continue;
					}
				}
				group.SetListCtrl(form);
				continue;
			}

			if (use_list) {
				group.SetList([this, &gnode](Bar& bar) {
					for (int k = 0; k < gnode.GetCount(); k++) {
						const XmlNode& cn = gnode.Node(k);
						if (!cn.IsTag("button"))
							continue;
						String id = NodeAttr(cn, "id");
						String text = NodeAttr(cn, "text");
						String sem = NodeAttr(cn, "icon_semantics");
						String icon = NodeAttr(cn, "icon");
						Image img = RibbonIconForId(id, sem, icon);
						bar.Add(text, [this, id] { OnAction(id); }).Image(img);
					}
				});
				continue;
			}

			group.SetLarge([this, &gnode](Bar& bar) {
				for (int k = 0; k < gnode.GetCount(); k++) {
					const XmlNode& cn = gnode.Node(k);
					if (!cn.IsTag("button"))
						continue;
					String id = NodeAttr(cn, "id");
					String text = NodeAttr(cn, "text");
					String sem = NodeAttr(cn, "icon_semantics");
					String icon = NodeAttr(cn, "icon");
					Image img = RibbonIconForId(id, sem, icon);
					bar.Add(text, [this, id] { OnAction(id); }).Image(img);
				}
			});
		}
	}
}

void RibbonCtrl::OnAction(const String& id)
{
	WhenAction(id);
	if (owner)
		owner->HandleRibbonAction(id);
}

Ctrl* RibbonCtrl::FindControl(const String& id) const
{
	int idx = control_by_id.Find(id);
	return idx >= 0 ? control_by_id[idx] : nullptr;
}

void RibbonCtrl::SetupQuickAccess()
{
	bar.SetQuickAccess([this](Bar& b) {
		b.Add(t_("New"), [this] { OnAction("new_file"); }).Image(CtrlImg::new_doc());
		b.Add(t_("Open"), [this] { OnAction("open_file"); }).Image(CtrlImg::open());
		b.Add(t_("Save"), [this] { OnAction("save_file"); }).Image(CtrlImg::save());
		b.Separator();
		b.Add(t_("Undo"), [this] { OnAction("undo"); }).Image(CtrlImg::undo());
		b.Add(t_("Redo"), [this] { OnAction("redo"); }).Image(CtrlImg::redo());
	});
}

void RibbonCtrl::AddContextTabs()
{
	RibbonPage& cam = bar.AddContextTab("camera", "Camera Tools");
	RibbonGroup& g = cam.AddGroup("Camera");
	g.SetLarge([this](Bar& b) {
		b.Add(t_("Make Active"), [this] { OnAction("camera_make_active"); }).Image(CtrlImg::smallcheck());
		b.Add(t_("Focus Selected"), [this] { OnAction("focus_selected"); }).Image(CtrlImg::arrow());
	});
}

END_UPP_NAMESPACE
