#include "ScriptIDE.h"

namespace Upp {

void PlotsPane::ImageDisplay::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, SColorFace());
	if(img.IsEmpty()) return;
	
	Size isz = img.GetSize();
	
	double ratio = min((double)sz.cx / isz.cx, (double)sz.cy / isz.cy);
	Size rsz = Size((int)(isz.cx * ratio), (int)(isz.cy * ratio));
	
	Point p = Point((sz.cx - rsz.cx) / 2, (sz.cy - rsz.cy) / 2);
	
	w.DrawImage(p.x, p.y, rsz.cx, rsz.cy, img);
}

PlotsPane::PlotsPane()
{
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(display.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void PlotsPane::LayoutToolbar(Bar& bar)
{
	bar.Add(plots.GetCount() > 1, "Previous", CtrlImg::left_arrow(), [=] { PrevPlot(); });
	bar.Add(plots.GetCount() > 1, "Next", CtrlImg::right_arrow(), [=] { NextPlot(); });
	bar.Separator();
	bar.Add(current_index >= 0, "Save As...", CtrlImg::save(), [=] { SaveSelected(); });
	bar.Add(current_index >= 0, "Copy", CtrlImg::copy(), [=] { CopySelected(); });
	bar.Separator();
	bar.Add(plots.GetCount() > 0, "Clear All", CtrlImg::remove(), [=] { Clear(); });
}

void PlotsPane::AddPlot(const Image& img)
{
	if(img.IsEmpty()) return;
	
	plots.Add(img);
	current_index = plots.GetCount() - 1;
	UpdateDisplay();
}

void PlotsPane::Clear()
{
	plots.Clear();
	current_index = -1;
	UpdateDisplay();
}

void PlotsPane::UpdateDisplay()
{
	if(current_index >= 0 && current_index < plots.GetCount()) {
		display.SetImage(plots[current_index]);
	}
	else {
		display.SetImage(Image());
	}
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void PlotsPane::PrevPlot()
{
	if(current_index > 0) {
		current_index--;
		UpdateDisplay();
	}
}

void PlotsPane::NextPlot()
{
	if(current_index < plots.GetCount() - 1) {
		current_index++;
		UpdateDisplay();
	}
}

void PlotsPane::SaveSelected()
{
	if(current_index < 0) return;
	
	FileSel fs;
	fs.Type("PNG Image", "*.png");
	fs.DefaultExt("png");
	if(fs.ExecuteSaveAs("Save Plot As")) {
		PNGEncoder().SaveFile(fs.Get(), plots[current_index]);
	}
}

void PlotsPane::CopySelected()
{
	if(current_index < 0) return;
	WriteClipboardImage(plots[current_index]);
}

}
