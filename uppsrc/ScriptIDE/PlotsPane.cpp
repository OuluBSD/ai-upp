#include "ScriptIDE.h"

namespace Upp {

PlotsPane::PlotsPane()
{
	Title("Plots");
	Icon(CtrlImg::help()); // Placeholder
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(display.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	UpdateDisplay();
}

void PlotsPane::LayoutToolbar(Bar& bar)
{
	bar.Add(CtrlImg::left_arrow(), [=] { PrevPlot(); }).Help("Previous plot");
	bar.Add(CtrlImg::right_arrow(), [=] { NextPlot(); }).Help("Next plot");
	bar.Separator();
	bar.Add(CtrlImg::save(), [=] { SaveSelected(); }).Help("Save plot");
	bar.Add(CtrlImg::save_as(), [=] { SaveAll(); }).Help("Save all plots");
	bar.Add(CtrlImg::copy(), [=] { CopySelected(); }).Help("Copy plot to clipboard");
	bar.Add(CtrlImg::remove(), [=] { RemoveSelected(); }).Help("Remove plot");
	bar.Add(CtrlImg::remove(), [=] { Clear(); }).Help("Remove all plots");
	bar.Separator();
	
	// Zoom controls
	bar.Add("Zoom:", [=] { Todo("Zoom percent"); });
	bar.Add(CtrlImg::plus(), [=] { Todo("Zoom in"); }).Help("Zoom in");
	bar.Add(CtrlImg::minus(), [=] { Todo("Zoom out"); }).Help("Zoom out");
	bar.Add("Fit to pane", [=] { Todo("Fit to pane"); });
	
	bar.Gap(2000);
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PlotsPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Mute inline plotting", [=] {}).Check(false);
	bar.Add("Show plot explorer", [=] {}).Check(true);
	bar.Separator();
	bar.Add("Move", [=] { Todo("Move pane"); });
	bar.Add("Undock", [=] { Todo("Undock pane"); });
	bar.Add("Close", [=] { Todo("Close pane"); });
}

void PlotsPane::AddPlot(const Image& img)
{
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
}

void PlotsPane::SaveSelected() { Todo("Save selected plot"); }
void PlotsPane::SaveAll() { Todo("Save all plots"); }
void PlotsPane::CopySelected() { Todo("Copy selected plot"); }
void PlotsPane::RemoveSelected()
{
	if(current_index >= 0 && current_index < plots.GetCount()) {
		plots.Remove(current_index);
		if(current_index >= plots.GetCount())
			current_index = plots.GetCount() - 1;
		UpdateDisplay();
	}
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

void PlotsPane::ImageDisplay::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, SColorPaper());
	if(!img.IsEmpty()) {
		Size isz = img.GetSize();
		w.DrawImage(0, 0, isz.cx * zoom, isz.cy * zoom, img);
	}
}

}
