#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Easing/Easing.h>

using namespace Upp;

// Small filled square used as the moving element for each curve's row.
struct Box : Ctrl {
	Color color = SBlack();

	virtual void Paint(Draw& w) {
		w.DrawRect(0, 0, GetSize().cx, GetSize().cy, color);
	}
};

// Faint background strip showing the travel path a box moves along.
struct Track : Ctrl {
	virtual void Paint(Draw& w) {
		w.DrawRect(0, 0, GetSize().cx, GetSize().cy, Color(228, 228, 228));
	}
};

struct EasingDemo : TopWindow {
	static constexpr int ROWS        = 4;
	static constexpr int DURATION_MS = 1500;
	static constexpr int BOX_SIZE    = 28;
	static constexpr int TRACK_LEFT  = 140;
	static constexpr int TRACK_RIGHT = 480;
	static constexpr int TOP_MARGIN  = 20;
	static constexpr int ROW_HEIGHT  = 50;

	struct CurveRow {
		const char*               name;
		Function<double(double)>  curve;
		Color                     color;
	};

	CurveRow rows[ROWS] = {
		{ "Linear",     EaseLinear,     Color(200,  60,  60) },
		{ "InCubic",    EaseInCubic,    Color( 60, 140, 200) },
		{ "OutCubic",   EaseOutCubic,   Color( 60, 180,  90) },
		{ "InOutCubic", EaseInOutCubic, Color(200, 140,  40) },
	};

	Label  labels[ROWS];
	Track  tracks[ROWS];
	Box    boxes[ROWS];
	Tween  tweens[ROWS];
	Button replay;

	EasingDemo()
	{
		Title("Ctrl/Easing demo: Tween curve comparison");
		CenterScreen().SetRect(0, 0, 540, TOP_MARGIN + ROWS * ROW_HEIGHT + 50);

		for(int i = 0; i < ROWS; i++) {
			int y = TOP_MARGIN + i * ROW_HEIGHT;

			Add(labels[i].SetLabel(rows[i].name).LeftPosZ(8, 120).TopPosZ(y + 4, 20));

			tracks[i].SetRect(TRACK_LEFT, y + BOX_SIZE / 2 - 2, TRACK_RIGHT - TRACK_LEFT, 4);
			Add(tracks[i]);

			boxes[i].color = rows[i].color;
			boxes[i].SetRect(TRACK_LEFT, y, BOX_SIZE, BOX_SIZE);
			Add(boxes[i]);
		}

		Add(replay.SetLabel("Replay").HSizePosZ(8, 8).BottomPosZ(10, 28));
		replay.WhenAction = [=] { StartAll(); };

		StartAll();
	}

	void StartAll()
	{
		for(int i = 0; i < ROWS; i++) {
			// Stop any tween still running from a previous press so a mid-animation
			// replay restarts cleanly instead of leaving stale callbacks racing in.
			tweens[i].Stop();

			int y = TOP_MARGIN + i * ROW_HEIGHT;
			boxes[i].SetRect(TRACK_LEFT, y, BOX_SIZE, BOX_SIZE);

			Function<double(double)> curve = rows[i].curve;
			tweens[i].Start(DURATION_MS, curve, [this, i, y](double v) {
				int x = TRACK_LEFT + int(v * (TRACK_RIGHT - TRACK_LEFT - BOX_SIZE));
				boxes[i].SetRect(x, y, BOX_SIZE, BOX_SIZE);
			});
		}
	}
};

GUI_APP_MAIN
{
	EasingDemo().Run();
}
