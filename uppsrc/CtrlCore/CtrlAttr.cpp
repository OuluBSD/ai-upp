#include "CtrlCore.h"

namespace Upp {

struct CtrlTransitionData {
	Ctrl::TransitionMode  mode = Ctrl::TRANSITION_NONE;
	Ctrl::TransitionCurve curve = Ctrl::TRANSITION_EASE_OUT_CUBIC;
	int                   duration = 350;
	double                progress = 1.0;
	bool                  running = false;
	TimeCallback          timer;
};

static double ApplyTransitionCurve(Ctrl::TransitionCurve curve, double t)
{
	t = clamp(t, 0.0, 1.0);
	switch (curve) {
	case Ctrl::TRANSITION_LINEAR: return t;
	case Ctrl::TRANSITION_EASE_IN_CUBIC: return t * t * t;
	case Ctrl::TRANSITION_EASE_IN_OUT_CUBIC:
		return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
	case Ctrl::TRANSITION_EASE_OUT_CUBIC:
	default:
		return 1 - pow(1 - t, 3);
	}
}

PackedData& Ctrl::Attrs()
{
	if(layout_id_literal) {
		String layout_id((const char *)attrs.GetRawPtr());
		attrs.SetRawPtr(nullptr);
		attrs.SetString(ATTR_LAYOUT_ID, layout_id);
		layout_id_literal = false;
	}
	return attrs;
}

void Ctrl::SetTextAttr(int ii, const char *s)
{
	Attrs().SetString(ii, s);
}

void Ctrl::SetTextAttr(int ii, const String& s)
{
	Attrs().SetString(ii, s);
}

String Ctrl::GetTextAttr(int ii) const
{
	return layout_id_literal ? String() : attrs.GetString(ii);
}

Ctrl& Ctrl::Tip(const char *txt)
{
	SetTextAttr(ATTR_TIP, txt);
	return *this;
}

Ctrl& Ctrl::HelpLine(const char *txt)
{
	SetTextAttr(ATTR_HELPLINE, txt);
	return *this;
}

Ctrl& Ctrl::Description(const char *txt)
{
	SetTextAttr(ATTR_DESCRIPTION, txt);
	return *this;
}

Ctrl& Ctrl::HelpTopic(const char *txt)
{
	SetTextAttr(ATTR_HELPTOPIC, txt);
	return *this;
}

Ctrl& Ctrl::LayoutId(const char *txt)
{
	SetTextAttr(ATTR_LAYOUT_ID, txt);
	return *this;
}

Ctrl& Ctrl::LayoutIdLiteral(const char *txt)
{
	if(attrs.GetRawPtr() && !layout_id_literal)
		LayoutId(txt);
	else {
		attrs.SetRawPtr((void *)txt);
		layout_id_literal = true;
	}
	return *this;
}

String Ctrl::GetLayoutId() const
{
	if(layout_id_literal)
		return (const char *)attrs.GetRawPtr();
	return GetTextAttr(ATTR_LAYOUT_ID);
}

String Ctrl::GetTip() const
{
	return GetTextAttr(ATTR_TIP);
}

String Ctrl::GetHelpLine() const
{
	return GetTextAttr(ATTR_HELPLINE);
}

String Ctrl::GetDescription() const
{
	return GetTextAttr(ATTR_DESCRIPTION);
}

String Ctrl::GetHelpTopic() const
{
	return GetTextAttr(ATTR_HELPTOPIC);
}

void Ctrl::ClearInfo()
{
	DeleteAttr<CtrlTransitionData>(ATTR_TRANSITION_DATA);
	if(layout_id_literal)
		attrs.SetRawPtr(nullptr);
	layout_id_literal = false;
	attrs.Clear();
}

void Ctrl::SetColorAttr(int ii, Color c)
{
	Attrs();
	if(IsNull(c))
		attrs.SetNull(ii);
	else
		attrs.SetDword(ii, c.GetRaw());
}

Color Ctrl::GetColorAttr(int ii) const
{
	if(layout_id_literal)
		return Null;
	static dword nullval = Color(Null).GetRaw();
	return Color::FromRaw(attrs.GetDword(ii, nullval));
}

void Ctrl::SetFontAttr(int ii, Font fnt)
{
	Attrs();
	if(IsNull(fnt))
		attrs.SetNull(ii);
	else
		attrs.SetInt64(ii, fnt.AsInt64());
}

Font Ctrl::GetFontAttr(int ii) const
{
	if(layout_id_literal)
		return Null;
	static int64 nullval = Font(Null).AsInt64();
	return Font::FromInt64(attrs.GetInt64(ii, nullval));
}

void Ctrl::SetIntAttr(int ii, int val)
{
	Attrs().SetInt(ii, val);
}

int Ctrl::GetIntAttr(int ii, int def) const
{
	if(layout_id_literal)
		return def;
	return attrs.GetInt(ii, def);
}

void Ctrl::SetInt64Attr(int ii, int64 val)
{
	Attrs().SetInt64(ii, val);
}

int64 Ctrl::GetInt64Attr(int ii, int64 def) const
{
	if(layout_id_literal)
		return def;
	return attrs.GetInt64(ii, def);
}


void Ctrl::SetVoidPtrAttr(int ii, const void *ptr)
{
	if(ptr)
		Attrs().SetPtr(ii, (void *)ptr);
	else
		Attrs().SetNull(ii);
}

void *Ctrl::GetVoidPtrAttr(int ii) const
{
	if(layout_id_literal)
		return NULL;
	return attrs.GetPtr(ii);
}

Ctrl& Ctrl::SetTransitionMode(TransitionMode mode)
{
	Attrs();
	CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data) {
		data = new CtrlTransitionData;
		attrs.SetPtr(ATTR_TRANSITION_DATA, data);
	}
	data->mode = mode;
	return *this;
}

Ctrl::TransitionMode Ctrl::GetTransitionMode() const
{
	if(layout_id_literal)
		return TRANSITION_NONE;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	return data ? data->mode : TRANSITION_NONE;
}

Ctrl& Ctrl::SetTransitionCurve(TransitionCurve curve)
{
	Attrs();
	CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data) {
		data = new CtrlTransitionData;
		attrs.SetPtr(ATTR_TRANSITION_DATA, data);
	}
	data->curve = curve;
	return *this;
}

Ctrl::TransitionCurve Ctrl::GetTransitionCurve() const
{
	if(layout_id_literal)
		return TRANSITION_EASE_OUT_CUBIC;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	return data ? data->curve : TRANSITION_EASE_OUT_CUBIC;
}

Ctrl& Ctrl::SetTransitionDuration(int ms)
{
	Attrs();
	CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data) {
		data = new CtrlTransitionData;
		attrs.SetPtr(ATTR_TRANSITION_DATA, data);
	}
	data->duration = max(1, ms);
	return *this;
}

int Ctrl::GetTransitionDuration() const
{
	if(layout_id_literal)
		return 350;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	return data ? data->duration : 350;
}

void Ctrl::StartTransition()
{
	GuiLock __;
	Attrs();
	CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data) {
		data = new CtrlTransitionData;
		attrs.SetPtr(ATTR_TRANSITION_DATA, data);
	}
	data->timer.Kill();
	data->progress = 0.0;
	data->running = true;
	data->timer.KillSet(16, [this] { TickTransition(); });
	Refresh();
}

void Ctrl::StopTransition()
{
	GuiLock __;
	if(layout_id_literal)
		return;
	if(CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA)) {
		data->timer.Kill();
		data->running = false;
		data->progress = 1.0;
	}
}

bool Ctrl::IsTransitionRunning() const
{
	if(layout_id_literal)
		return false;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	return data && data->running;
}

double Ctrl::GetTransitionProgress() const
{
	if(layout_id_literal)
		return 1.0;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	return data ? data->progress : 1.0;
}

double Ctrl::GetTransitionValue() const
{
	if(layout_id_literal)
		return 1.0;
	const CtrlTransitionData *data = (const CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data)
		return 1.0;
	return ApplyTransitionCurve(data->curve, data->progress);
}

void Ctrl::TickTransition()
{
	GuiLock __;
	if(layout_id_literal)
		return;
	CtrlTransitionData *data = (CtrlTransitionData *)attrs.GetPtr(ATTR_TRANSITION_DATA);
	if(!data || !data->running)
		return;

	double step = 16.0 / max(1, data->duration);
	data->progress = min(1.0, data->progress + step);
	Refresh();
	if(data->progress >= 1.0) {
		data->running = false;
		data->timer.Kill();
	}
	else
		data->timer.KillSet(16, [this] { TickTransition(); });
}

};
