#include "ScriptIDE.h"

namespace Upp {

PlotsPane::PlotsPane()
{
	Title("Plots");
	Icon(CtrlImg::plus());
	
	Add(toolbar.TopPos(0, 24).HSizePos());
	Add(display.VSizePos(24, 0).HSizePos());
	
	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });
}

void PlotsPane::LayoutToolbar(Bar& bar)
{
	bar.Add("Save plot as...", CtrlImg::save_as(), [=] { SaveSelected(); }).Help("Save plot as...");
	bar.Add("Save all plots...", CtrlImg::save_as(), [=] { SaveAll(); }).Help("Save all plots...");
	bar.Add("Copy plot to clipboard as image", CtrlImg::copy(), [=] { CopySelected(); }).Help("Copy to clipboard");
	bar.Add("Remove plot", CtrlImg::remove(), [=] { RemoveSelected(); }).Help("Remove current plot");
	bar.Add("Remove all plots", CtrlImg::remove(), [=] { Clear(); }).Help("Remove all plots");
	bar.Separator();
	bar.Add("Zoom percent", [=] { Todo("Zoom dropdown"); });
	bar.Add("Zoom out", [=] { display.zoom /= 1.1; display.Refresh(); }).Help("Zoom out");
	bar.Add("Zoom in", [=] { display.zoom *= 1.1; display.Refresh(); }).Help("Zoom in");
	bar.Add("Fit plot to pane size", [=] { display.zoom = 1.0; display.Refresh(); }).Help("Fit to pane");
	bar.Gap(2000);
	bar.Add(CtrlImg::left_arrow(), [=] { PrevPlot(); }).Help("Previous plot");
	bar.Add(CtrlImg::right_arrow(), [=] { NextPlot(); }).Help("Next plot");
	bar.Sub("Options", CtrlImg::plus(), [=](Bar& b) { LayoutPaneMenu(b); });
}

void PlotsPane::LayoutPaneMenu(Bar& bar)
{
	bar.Add("Mute inline plotting", [=] { Todo("Mute plots"); }).Check(false);
	bar.Add("Show plot outline", [=] { Todo("Plot outline"); }).Check(false);
	bar.Add("Set maximum number of plots...", [=] { Todo("Max plots"); });
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
	if(current_index >= 0 && current_index < plots.GetCount())
		display.SetImage(plots[current_index]);
	else
		display.SetImage(Image());
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
	fs.Type("PNG image", "*.png");
	if(fs.ExecuteSaveAs("Save Plot")) {
		PNGEncoder().SaveFile(fs.Get(), plots[current_index]);
	}
}

void PlotsPane::SaveAll()
{
	if(plots.IsEmpty()) return;
	FileSel fs;
	if(fs.ExecuteSelectDir("Select Directory to Save Plots")) {
		String dir = fs.Get();
		for(int i = 0; i < plots.GetCount(); i++) {
			PNGEncoder().SaveFile(AppendFileName(dir, "plot_" + AsString(i) + ".png"), plots[i]);
		}
	}
}

void PlotsPane::CopySelected() { if(current_index >= 0) WriteClipboardImage(plots[current_index]); }
void PlotsPane::RemoveSelected()
{
	if(current_index >= 0) {
		plots.Remove(current_index);
		if(current_index >= plots.GetCount()) current_index = plots.GetCount() - 1;
		UpdateDisplay();
	}
}

void PlotsPane::ImageDisplay::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, SColorPaper());
	if(!img.IsEmpty()) {
		Size isz = img.GetSize();
		isz.cx = (int)(isz.cx * zoom);
		isz.cy = (int)(isz.cy * zoom);
		w.DrawImage((sz.cx - isz.cx) / 2, (sz.cy - isz.cy) / 2, isz.cx, isz.cy, img);
	}
}

}
