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
	Title("Plots");
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(display.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void PlotsPane::LayoutToolbar(Bar& bar)
{
	bar.Add(current_index >= 0, CtrlImg::save(), [=] { SaveSelected(); }).Help("Save plot as...");
	bar.Add(plots.GetCount() > 0, CtrlImg::save(), [=] { SaveAll(); }).Help("Save all plots...");
	bar.Add(current_index >= 0, CtrlImg::copy(), [=] { CopySelected(); }).Help("Copy to clipboard");
	bar.Add(current_index >= 0, CtrlImg::remove(), [=] { RemoveSelected(); }).Help("Remove plot");
	bar.Add(plots.GetCount() > 0, CtrlImg::remove(), [=] { Clear(); }).Help("Remove all plots");
	bar.Separator();
	bar.Add("Zoom", [=] {});
	bar.Add(CtrlImg::plus(), [=] {}).Help("Zoom in");
	bar.Add(CtrlImg::remove(), [=] {}).Help("Zoom out");
	bar.Add("Fit", [=] {}).Help("Fit plot to pane");
	bar.Separator();
	bar.Gap(2000);
	bar.Add(plots.GetCount() > 1, CtrlImg::left_arrow(), [=] { PrevPlot(); }).Help("Previous plot");
	bar.Add(plots.GetCount() > 1, CtrlImg::right_arrow(), [=] { NextPlot(); }).Help("Next plot");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PlotsPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Mute inline plotting", [=] {}).Check(false);
	bar.Add("Show plot outline", [=] {}).Check(false);
	bar.Add("Set maximum number of plots...", [=] {});
}

void PlotsPane::SaveAll() { PromptOK("Todo: Save all plots"); }

void PlotsPane::RemoveSelected()
{
	if(current_index >= 0 && current_index < plots.GetCount()) {
		plots.Remove(current_index);
		if(current_index >= plots.GetCount())
			current_index = plots.GetCount() - 1;
		UpdateDisplay();
	}
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
