Task 0008 - StereoCalibrationTool GUI layout plan

Layout summary
- Window: MenuBar + StatusBar + main Vert splitter.
- Main split: vsplitter.Vert(hsplitter, bottom_tabs).
- hsplitter.Horz(left, right), hsplitter.SetPos(2000) (~20% left).

Left column (ParentCtrl)
- Controls laid out with HSizePos/TopPos.
- Sections separated by LabelBox or SeparatorCtrl:
  1) Source controls: DropList, Start/Stop, Live view, Capture, status label.
  2) Mode selector: point-pairs vs line-pairs (in left column; optionally also in menu).
  3) Calibration fields: Enabled, Eye dist, Outward angle, Angle poly A/B/C/D.
  4) Diagnostics: samples count, coverage meter, min/max distance, calculated summary.
- Optional: vertical scrollbar if controls exceed height.

Right column (Preview Ctrl)
- Custom Ctrl subclass with Paint(Draw&).
- Split rendering (left/right) + overlay drawing (points/lines/ids).
- Capture freezes preview to snapshot until Live view resumes.

Bottom tabs (TabCtrl)
- Tab: Captured Frames (ArrayCtrl list of snapshots; selection shows in preview).
- Tab: Matches (ArrayCtrl of point/line pairs for current snapshot).
- Tab: Report (text preview .stcal + solve report + current snapshot computed values).

Notes
- Mode selector must be in left column or menu (or both).
- StatusBar used as bottom strip (AddFrame(status)).
