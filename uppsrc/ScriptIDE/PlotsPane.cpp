#include "ScriptIDE.h"

namespace Upp {

PlotsPane::PlotsPane()
{
	Title("Plots");
	Icon(Icons::Plots());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(display.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
	
	UpdateDisplay();
}

void PlotsPane::LayoutToolbar(Bar& bar)
{
	bar.Add(Icons::Undo(), [=] { PrevPlot(); }).Help("Previous plot");
	bar.Add(Icons::Redo(), [=] { NextPlot(); }).Help("Next plot");
	bar.Separator();
	bar.Add(Icons::Save(), [=] { SaveSelected(); }).Help("Save plot");
	bar.Add(Icons::SaveAll(), [=] { SaveAll(); }).Help("Save all plots");
	bar.Add(Icons::Undo(), [=] { CopySelected(); }).Help("Copy plot to clipboard");
	bar.Add(Icons::Stop(), [=] { RemoveSelected(); }).Help("Remove plot");
	bar.Add(Icons::Stop(), [=] { Clear(); }).Help("Remove all plots");
	bar.Separator();
	
	// Zoom controls
	bar.Add("Zoom:", [=] { Todo("Zoom percent"); });
	bar.Add(Icons::Plus(), [=] { Todo("Zoom in"); }).Help("Zoom in");
	bar.Add(Icons::Stop(), [=] { Todo("Zoom out"); }).Help("Zoom out"); // Should be minus
	bar.Add("Fit to pane", [=] { Todo("Fit to pane"); });
	
	bar.Gap(2000);
	bar.Sub("Options", Icons::Settings(), [=](Bar& b) { LayoutPaneMenu(b); });
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
