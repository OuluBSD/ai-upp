#include "Common/FormCommon.h"
#include "Form.hpp"
#include "Common/IniConfig.hpp"

static int tempFormsCount = 0;

static Ctrl::TransitionMode ParseTransitionMode(const String& s)
{
	if (s == "Fade")
		return Ctrl::TRANSITION_FADE;
	if (s == "Flip horizontal")
		return Ctrl::TRANSITION_FLIP_H;
	if (s == "Flip vertical")
		return Ctrl::TRANSITION_FLIP_V;
	return Ctrl::TRANSITION_NONE;
}

static VectorMap<String, FormCtrlFactory>& GetFormCtrlFactories()
{
	static VectorMap<String, FormCtrlFactory> factories;
	return factories;
}

INITBLOCK {
	RegisterFormCtrlType<DropList>("DropList");
}

void RegisterFormCtrlFactory(const String& type, FormCtrlFactory factory)
{
	if (type.IsEmpty() || !factory)
		return;
	VectorMap<String, FormCtrlFactory>& factories = GetFormCtrlFactories();
	int pos = factories.Find(type);
	if (pos >= 0)
		factories[pos] = factory;
	else
		factories.Add(type, factory);
}

static Ctrl::TransitionCurve ParseTransitionCurve(const String& s)
{
	if (s == "Linear")
		return Ctrl::TRANSITION_LINEAR;
	if (s == "Ease in cubic")
		return Ctrl::TRANSITION_EASE_IN_CUBIC;
	if (s == "Ease in/out cubic")
		return Ctrl::TRANSITION_EASE_IN_OUT_CUBIC;
	return Ctrl::TRANSITION_EASE_OUT_CUBIC;
}

static Form::ScaleMode ParseScaleMode(const String& s)
{
	if (s == "Fit")
		return Form::SCALE_FIT;
	if (s == "Relative")
		return Form::SCALE_RELATIVE;
	return Form::SCALE_NONE;
}

static Color LoadFormColor(const FormObject& object, const char *key, Color fallback)
{
	String data = object.Get(key);
	if (data.IsEmpty())
		return fallback;
	if (data.Find('#') >= 0 || data.Find(',') >= 0 || data.Find(';') >= 0 || data.Find('.') >= 0)
		return Nvl(ColorFromText(data), fallback);
	LoadFromString(fallback, Decode64(data));
	return fallback;
}

static Image LoadFormImage(const String& form_file, const String& path)
{
	if (path.IsEmpty())
		return Null;

	String resolved = path;
	if (!IsFullPath(resolved) && !form_file.IsEmpty())
		resolved = AppendFileName(GetFileDirectory(form_file), resolved);

	return StreamRaster::LoadFileAny(resolved);
}

Form:: Form() : _Current(-1), _ScaleMode(SCALE_NONE) {}
Form::~Form() { Clear(); }

bool Form::Load(const String& file)
{
	Clear();

	if (!FileExists(file))
		return false;

	if (GetFileExt(file) == ".fz")
	{
		String s = ZDecompress(LoadFile(file));
		if (!LoadFromXML(*this, s))
			return false;
		_File = file;
		return true;
	}

	if (!LoadFromXMLFile(*this, file))
		return false;
	_File = file;
	return true;
}

bool Form::LoadString(const String& xml, bool compression)
{
	Clear();
	_File = "";
	
	if (compression)
	{
		String s = ZDecompress(xml);
		if (!LoadFromXML(*this, s))
			return false;
		return true;
	}

	if (!LoadFromXML(*this, xml))
		return false;
	
	return true;
}

bool Form::Layout(const String& layout, Font font)
{
	_Current = -1;
	for (int i = 0; i < _Layouts.GetCount(); ++i)
		if (_Layouts[i].Get("Form.Name") == layout)
		{
			_Current = i;
			String scale_mode = _Layouts[i].Get("Form.ScaleMode");
			if(!scale_mode.IsEmpty())
				SetScaleMode(ParseScaleMode(scale_mode));
			return Generate(font);
	}
	return false;
}

void Form::Layout()
{
	if(!IsLayout() || _Ctrls.IsEmpty())
		return;

	if(_ScaleMode != SCALE_FIT && _ScaleMode != SCALE_RELATIVE) {
		for(Ctrl& ctrl : _Ctrls)
			if(Form* form = dynamic_cast<Form*>(&ctrl))
				if(form->GetScaleMode() == SCALE_FIT || form->GetScaleMode() == SCALE_RELATIVE)
					form->Layout();
		return;
	}

	FormLayout& layout = _Layouts[_Current];
	Size base = layout.GetFormSize();
	Size size = GetSize();
	if(base.cx <= 0 || base.cy <= 0 || size.cx <= 0 || size.cy <= 0)
		return;

	if(_ScaleMode == SCALE_RELATIVE) {
		for(Ctrl& ctrl : _Ctrls)
			if(Form* form = dynamic_cast<Form*>(&ctrl))
				form->Layout();
		return;
	}

	double scale = min((double)size.cx / base.cx, (double)size.cy / base.cy);
	int scaled_cx = max(1, int(base.cx * scale + 0.5));
	int scaled_cy = max(1, int(base.cy * scale + 0.5));
	int offset_x = (size.cx - scaled_cx) / 2;
	int offset_y = (size.cy - scaled_cy) / 2;

	Array<FormObject>& objects = layout.GetObjects();
	for(int i = 0; i < objects.GetCount() && i < _Ctrls.GetCount(); ++i) {
		Rect r = objects[i].GetRect();
		r.left   = offset_x + int(r.left   * scale + 0.5);
		r.top    = offset_y + int(r.top    * scale + 0.5);
		r.right  = offset_x + int(r.right  * scale + 0.5);
		r.bottom = offset_y + int(r.bottom * scale + 0.5);
		_Ctrls[i].SetRect(r);
	}

	Refresh();
}

bool Form::Generate(Font font)
{
	if (!IsLayout())
		return false;

	Clear(false);

	Size sz = _Layouts[_Current].GetFormSize();
	if(!GetParent())
		SetRect(Rect(Point(0, 0), Size(HorzLayoutZoom(sz.cx), VertLayoutZoom(sz.cy))));

	Array<FormObject>* p = &_Layouts[_Current].GetObjects();

	for (int i = 0; i < p->GetCount(); ++i)
	{
		Font objFont = font;
		int h = (*p)[i].GetNumber("Font.Height", 0);
		if (h != 0) objFont.Height( VertLayoutZoom(h) );
		if (font.GetHeight() == 0) objFont.Height( StdFont().GetHeight() );
		Ctrl *c = NULL;

		if ((*p)[i].Get("Type") == "Button")
		{
			Button *b = &_Ctrls.Create<Button>((*p)[i].Get("Variable"));
			b->SetFont(objFont);
			b->SetLabel((*p)[i].Get("Label"));
			b->Tip((*p)[i].Get("Tip"));
			*b <<= THISBACK1(OnAction, (*p)[i].Get("Action"));
			c = b;
		}

		if ((*p)[i].Get("Type") == "StaticRect")
		{
			StaticRect *r = &_Ctrls.Create<StaticRect>((*p)[i].Get("Variable"));
			r->Background(LoadFormColor((*p)[i], "StaticRect.Background", SColorFace()));
			c = r;
		}

		if ((*p)[i].Get("Type") == "ImageCtrl")
		{
			ImageCtrl *img = &_Ctrls.Create<ImageCtrl>((*p)[i].Get("Variable"));
			img->SetImage(LoadFormImage(_File,
				(*p)[i].Get("ImageCtrl.Image.Path", (*p)[i].Get("Image.Path"))));
			c = img;
		}

		if ((*p)[i].Get("Type") == "EditField")
		{
			EditField *e = &_Ctrls.Create<EditField>((*p)[i].Get("Variable"));
			e->SetFont(objFont);
			if ((*p)[i].Get("Style") == "Password")
				e->Password();
			e->Tip((*p)[i].Get("Tip"));
			if ((*p)[i].Get("TextAlign") == "Right")
				e->AlignRight();

			c = e;
		}

		if ((*p)[i].Get("Type") == "Label")
		{
			Label *e = &_Ctrls.Create<Label>((*p)[i].Get("Variable"));
			e->SetFont(objFont);
			Color fontColor = SColorText();
			String font_color_text = (*p)[i].Get("Font.Color", StoreAsString(fontColor));
			Color parsed_color = ColorFromText(font_color_text);
			if(IsNull(parsed_color))
				LoadFromString(fontColor, Decode64(font_color_text));
			else
				fontColor = parsed_color;
			e->SetInk(fontColor);
			String align = (*p)[i].Get("Text.Align");
			if (align == "Center") e->SetAlign(ALIGN_CENTER);
			if (align == "Right") e->SetAlign(ALIGN_RIGHT);
			if (align == "Left") e->SetAlign(ALIGN_LEFT);
			e->SetLabel((*p)[i].Get("Label"));
			c = e;
		}

		if ((*p)[i].Get("Type") == "DropDate")
		{
			DropDate *e = &_Ctrls.Create<DropDate>((*p)[i].Get("Variable"));
			e->SetFont(objFont);
			c = e;
		}

		if ((*p)[i].Get("Type") == "ProgressBar")
		{
			ProgressIndicator *e = &_Ctrls.Create<ProgressIndicator>((*p)[i].Get("Variable"));
			e->Set(
				(*p)[i].GetNumber("ProgressBar.Initial", 0),
				(*p)[i].GetNumber("ProgressBar.Total", 100));
			c = e;
		}

		if ((*p)[i].Get("Type") == "EditInt")
		{
			EditInt *e = &_Ctrls.Create<EditInt>((*p)[i].Get("Variable"));
			e->SetFont(objFont);
			e->Min((*p)[i].GetNumber("Min", INT_MIN, INT_MAX, INT_MIN));
			e->Max((*p)[i].GetNumber("Max", INT_MIN, INT_MAX, INT_MAX));
			c = e;
		}

		if ((*p)[i].Get("Type") == "GridCtrl")
		{
			GridCtrl *e = &_Ctrls.Create<GridCtrl>((*p)[i].Get("Variable"));
			e->Chameleon();
			c = e;

			String src = (*p)[i].Get("Grid.Columns");
			ReplaceString(src, ";", "\r\n");
			StringStream s;
			s.Open(src);
			IniFile f;
			f.Load(s);

			Vector<String> names = f.EnumNames("Columns");

			for (int j = 0; j < names.GetCount(); ++j)
			{
				int n = ScanInt(names[j]);

				if (AsString(n) != names[j])
					continue;

				Vector<String> values = f.GetArray("Columns", names[j]);
				if (values.GetCount() != 3)
					continue;
				if (values[1] == "Left") e->AddColumn(values[0]).HeaderAlignCenterLeft();
				else if (values[1] == "Right") e->AddColumn(values[0]).HeaderAlignCenterRight();
				else e->AddColumn(values[0]).HeaderAlignCenter();
			}
		}

		if ((*p)[i].Get("Type") == "TabCtrl")
		{
			TabCtrl *e = &_Ctrls.Create<TabCtrl>((*p)[i].Get("Variable"));
			TabCtrl::Style& style = e->StyleDefault().Write();
			style.font = objFont;
			style.tabheight = objFont.GetHeight() + VertLayoutZoom(10);
			e->SetStyle(style);

			String src = (*p)[i].Get("Tab.Content");
			ReplaceString(src, ";", "\r\n");
			StringStream s;
			s.Open(src);
			IniFile f;
			f.Load(s);

			Vector<String> names = f.EnumNames("Tabs");
			VectorMap<int, Vector<String> > cache;

			for (int j = 0; j < names.GetCount(); ++j)
			{
				int n = ScanInt(names[j]);

				if (AsString(n) != names[j])
					continue;

				Vector<String> values = f.GetArray("Tabs", names[j]);
				if (values.GetCount() != 3)
					continue;

				if (values[0] == t_("Current form"))
					values[0] = _File;

				Container *cont = &_Ctrls.Create<Container>("TemporaryContainer"
					+ AsString(tempFormsCount));

				Form *f = &_Ctrls.Create<Form>("TemporaryForm" + AsString(tempFormsCount++));

				if (values[0] != _File)
				{
					if (!f->Load(GetFileDirectory(_File) + "\\" + values[0]))
						continue;
				}
				else
				{
					int lay = HasLayout(values[1]);
					if (lay < 0)
						continue;
					f->GetLayouts().Add(_Layouts[lay]);
				}

				if (!f->Layout(values[1], objFont))
					continue;

				cont->Set(*f, f->GetSize());
				cont->SizePos();
				e->Add(*cont, values[2]);
			}

			if (e->GetCount())
			{
				int active = (*p)[i].GetNumber("Tab.Active", -1, -1);

				if (active < 0)
					active = 0;

				if (active >= e->GetCount())
					active  = e->GetCount() - 1;

				e->Set(active);
			}

			c = e;
		}

		if ((*p)[i].Get("Type") == "Form")
		{
			Form *f = &_Ctrls.Create<Form>((*p)[i].Get("Variable"));
			f->SignalHandler = SignalHandler;
			String path = (*p)[i].Get("Form.Path");
			(*p)[i].Get("Form.PathType") == "Relative"
				? f->Load(::GetFileDirectory(_File) + "\\" + path)
				: f->Load(path);
			f->SetScaleMode(ParseScaleMode((*p)[i].Get("Form.ScaleMode")));
			f->Layout((*p)[i].Get("Form.Layout"), objFont);
			f->Script = Script;
			c = f;
		}

		if(!c) {
			String type = (*p)[i].Get("Type");
			VectorMap<String, FormCtrlFactory>& factories = GetFormCtrlFactories();
			int pos = factories.Find(type);
			if(pos >= 0)
				c = factories[pos](_Ctrls, (*p)[i].Get("Variable"));
		}

		if(c) {
			c->SetTransitionMode(ParseTransitionMode((*p)[i].Get("Transition.Mode")));
			c->SetTransitionCurve(ParseTransitionCurve((*p)[i].Get("Transition.Easing")));
			c->SetTransitionDuration((*p)[i].GetNumber("Transition.Duration", 350));
		}

		if (!c)
			continue;
	}

	for (int i = 0; i < p->GetCount(); ++i)
	{
		Ctrl& c = _Ctrls[i];
		Rect obj_rect = (*p)[i].GetRect();
		dword h_align = (*p)[i].GetHAlign();
		dword v_align = (*p)[i].GetVAlign();
		String anchor = (*p)[i].Get("Anchor");
		if(!anchor.IsEmpty())
			ResolveAnchorLayout(obj_rect, h_align, v_align, anchor, sz);

		if(_ScaleMode == SCALE_RELATIVE && sz.cx > 0 && sz.cy > 0)
			c.SizeRel((double)obj_rect.left / sz.cx, (double)obj_rect.top / sz.cy,
			          (double)(sz.cx - obj_rect.right) / sz.cx,
			          (double)(sz.cy - obj_rect.bottom) / sz.cy);
		else {
			switch(h_align)
			{
				case Ctrl::LEFT:
					c.LeftPosZ(obj_rect.left, obj_rect.Width());
					break;
				case Ctrl::RIGHT:
					c.RightPosZ(obj_rect.left, obj_rect.Width());
					break;
				case Ctrl::SIZE:
					c.HSizePosZ(obj_rect.left, sz.cx - obj_rect.right);
					break;
				case Ctrl::CENTER:
					c.HCenterPosZ(obj_rect.Width(), obj_rect.left);
					break;
			}

			switch(v_align)
			{
				case Ctrl::TOP:
					c.TopPosZ(obj_rect.top, obj_rect.Height());
					break;
				case Ctrl::BOTTOM:
					c.BottomPosZ(obj_rect.top, obj_rect.Height());
					break;
				case Ctrl::SIZE:
					c.VSizePosZ(obj_rect.top, sz.cy - obj_rect.bottom);
					break;
				case Ctrl::CENTER:
					c.VCenterPosZ(obj_rect.Height(), obj_rect.top);
					break;
			}
		}

		String frame = (*p)[i].Get("Frame");

		if (frame == "Null frame")             c.SetFrame(NullFrame());
		if (frame == "Field frame")            c.SetFrame(FieldFrame());
		if (frame == "Inset frame")            c.SetFrame(InsetFrame());
		if (frame == "Outset frame")           c.SetFrame(OutsetFrame());
		if (frame == "Thin inset frame")       c.SetFrame(ThinInsetFrame());
		if (frame == "Thin outset frame")      c.SetFrame(ThinOutsetFrame());
		if (frame == "Black frame")            c.SetFrame(BlackFrame());
		if (frame == "Button frame")           c.SetFrame(ButtonFrame());
		if (frame == "Top separator frame")    c.SetFrame(TopSeparatorFrame());
		if (frame == "Left separator frame")   c.SetFrame(LeftSeparatorFrame());
		if (frame == "Right separator frame")  c.SetFrame(RightSeparatorFrame());
		if (frame == "Bottom separator frame") c.SetFrame(BottomSeparatorFrame());
	}

	for (int i = 0; i < p->GetCount(); ++i)
	{
		Ctrl& c = _Ctrls[i];
		String parent = (*p)[i].Get("Parent");
		if(!parent.IsEmpty()) {
			if(Ctrl* parent_ctrl = GetCtrl(parent)) {
				parent_ctrl->Add(c);
				if(_ScaleMode == SCALE_RELATIVE) {
					Size parent_size;
					for(int j = 0; j < p->GetCount(); j++)
						if((*p)[j].Get("Variable") == parent) {
							parent_size = (*p)[j].GetRect().GetSize();
							break;
						}
					if(parent_size.cx > 0 && parent_size.cy > 0) {
						Rect child_rect = (*p)[i].GetRect();
						c.SizeRel((double)child_rect.left / parent_size.cx,
						          (double)child_rect.top / parent_size.cy,
						          (double)(parent_size.cx - child_rect.right) / parent_size.cx,
						          (double)(parent_size.cy - child_rect.bottom) / parent_size.cy);
					}
				}
				continue;
			}
		}
		Add(c);
	}

	Layout();
	WhenGenerate();
	
	return true;
}

String FormWindow::ExecuteForm()
{
	if (!form.IsLayout())
		return "Error";

	WhenClose = TopWindow::Breaker(0);
	int result;

	for (;;)
	{
		result = TopWindow::Execute();

		if (result == 0)
			return "Close";

		if (result < 0)
			return form._Rejectors[ abs(result) - 1 ];

		Array<FormObject>* p = &form._Layouts[form._Current].GetObjects();
		bool null = false;
		for (int i = 0; i < p->GetCount(); ++i)
		{
			if ((*p)[i].GetBool("NotNull"))
				if (IsNull(~form._Ctrls[i]))
				{
					PromptOK("Необходимо заполнить поле: " + (*p)[i].Get("Variable"));
					null = true;
					break;
				}
		}
		if (null)
			continue;

		break;
	}

	return form._Acceptors[ result - 1 ];
}

void Form::Clear(bool all)
{
	if (all) {
		_Layouts.Clear();
		_ScaleMode = SCALE_NONE;
	}
	_Rejectors.Clear();
	_Acceptors.Clear();
	_Ctrls.Clear();
	Script.Clear();
}

int Form::HasLayout(const String& layout)
{
	for (int i = 0; i < _Layouts.GetCount(); ++i)
		if (_Layouts[i].Get("Form.Name") == layout)
			return i;
	return -1;
}

void Form::OnAction(const String& action)
{
	SignalHandler(Script, "OnAction", action);
}

bool FormWindow::Exit(const String& action)
{
	return form.SetCallback(action, TopWindow::Rejector(0));
}

bool FormWindow::Acceptor(const String& action)
{
	if (form.SetCallback(action, TopWindow::Acceptor( form._Acceptors.GetCount() + 1 )))
	{
		form._Acceptors << action;
		return true;
	}
	return false;
}

bool FormWindow::Rejector(const String& action)
{
	if (form.SetCallback(action, TopWindow::Rejector( -(form._Rejectors.GetCount() + 1) )))
	{
		form._Rejectors << action;
		return true;
	}
	return false;
}

bool Form::SetCallback(const String& action, Callback c)
{
	if (!IsLayout())
		return false;

	Array<FormObject>* p = &_Layouts[_Current].GetObjects();
	for (int i = 0; i < p->GetCount(); ++i)
	{
		if ((*p)[i].Get("Type") == "Button")
			if ((*p)[i].Get("Action") == action)
			{
				_Ctrls[i] <<= c;
				return true;
			}
	}

	return false;
}

Value Form::GetData(const String& var)
{
	if (!IsLayout())
		return false;

	Array<FormObject>* p = &_Layouts[_Current].GetObjects();
	for (int i = 0; i < p->GetCount(); ++i)
	{
		if ((*p)[i].Get("Variable") == var)
			return ~_Ctrls[i];
	}
	return Value();
}

Ctrl* Form::GetCtrl(const String& var)
{
	if (!IsLayout())
		return NULL;

	Array<FormObject>* p = &_Layouts[_Current].GetObjects();
	for (int i = 0; i < p->GetCount(); ++i)
	{
		if ((*p)[i].Get("Variable") == var)
			return &_Ctrls[i];
	}
	return NULL;
}


FormWindow::FormWindow() {
	Add(form.SizePos());
	form.WhenGenerate << [this]{this->Generate();};
}

void FormWindow::Generate() {
	auto& l = form._Layouts[form._Current];
	
	if (preview_chrome || l.GetBool("Form.MinimizeBox", false)) MinimizeBox();
	if (preview_chrome || l.GetBool("Form.MaximizeBox", false)) MaximizeBox();

#ifdef PLATFORM_WIN32
	if (!preview_chrome && l.GetBool("Form.ToolWindow", false)) ToolWindow();
#endif

	if (preview_chrome || l.GetBool("Form.Sizeable", false)) Sizeable();

	Title(l.Get("Form.Title"));

	Size sz = l.GetFormSize();
	SetRect(Rect(GetRect().TopLeft(), Size(HorzLayoutZoom(sz.cx), VertLayoutZoom(sz.cy))));
	form.SizePos();
}
