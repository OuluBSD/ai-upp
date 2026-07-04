#include <Core/Core.h>
using namespace Upp;

#include <Form/Form.hpp>

// Preview-only stand-in for the CtrlCore Easing/Compositing/Transforms tracks (none of
// which exist yet, see plan/CompositeEasingNetwork): a hand-rolled ease-out tween driven
// by a repeating TimeCallback, animating a real themed, alpha-channel card asset
// (share/imgs/cards/default/, the same theme directory game/TexasHoldem's GameTable uses)
// in via one of several distinct transition styles - fade+move, 2D rotation, three
// different flip-style fake-3D turns (each showing the card back first, then revealing the
// face partway through, like an actual card flip), and a wavy "magic carpet" unfurl -
// picked at random, from a random direction, on every deal. All of it is done with today's
// Draw/Painter primitives (Xform2D, DrawPainter, per-pixel alpha); nothing here depends on
// the not-yet-built Compositing/Easing/Transforms tracks.
class CardArt : public Ctrl {
	typedef CardArt CLASSNAME;

	enum Effect { FADE_MOVE, ROTATE_2D, FLIP_H, FLIP_V, FLIP_DIAG, MAGIC_CARPET, EFFECT_COUNT };

	int          card_index = -1;  // -1 = card back, 0..51 = suit*13 + rank
	double       anim       = 1.0; // 0 = just dealt, 1 = settled
	int          effect     = FADE_MOVE;
	int          dir        = 0;   // 0..3, meaning depends on the effect
	TimeCallback tick;

	static double EaseOutCubic(double t)
	{
		double u = 1 - t;
		return 1 - u * u * u;
	}

	static Image LoadCard(int index)
	{
		static const char *suits[] = { "clubs", "diamonds", "hearts", "spades" };
		static const char *ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
		String filename = index < 0 ? String("back1.png")
		                            : String(suits[index / 13]) + "_" + ranks[index % 13] + ".png";
		return StreamRaster::LoadFileAny(ShareDirFile("imgs/cards/default/" + filename));
	}

	// Multiplies the theme PNG's own per-pixel alpha (rounded corners etc.) by a uniform
	// factor - demonstrates fading real transparent artwork, not just a synthetic shape.
	static Image Faded(Image src, double alpha01)
	{
		ImageBuffer ib(src);
		int mul = int(clamp(alpha01, 0.0, 1.0) * 255);
		for(int y = 0; y < ib.GetHeight(); y++) {
			RGBA *p = ib[y];
			for(int x = 0; x < ib.GetWidth(); x++, p++)
				p->a = byte((int(p->a) * mul) / 255);
		}
		return ib;
	}

	static void PaintShadow(Draw& w, Size sz, Rect dest, double alpha)
	{
		DrawPainter dp(w, sz);
		dp.Clear(RGBAZero());
		RGBA c;
		c.r = 0; c.g = 0; c.b = 0; c.a = byte(clamp(alpha, 0.0, 1.0) * 90);
		dp.RoundedRectangle(dest.left + 6, dest.top + 8, dest.Width(), dest.Height(), 10).Fill(c);
	}

	// Two-phase "turn over": the back shrinks away (phase 1, e in [0, 0.5)), then the face
	// grows back in (phase 2, e in [0.5, 1]) - the classic 2D trick for a card-flip illusion.
	// squeeze() returns the 0..1 foreshortening factor to apply to width and/or height.
	Image FlipFace(double e, double& squeeze) const
	{
		if(e < 0.5) {
			squeeze = 1 - e * 2;
			return LoadCard(-1);
		}
		squeeze = (e - 0.5) * 2;
		return LoadCard(card_index);
	}

	// A uniform width/height squeeze alone (squeeze applied equally at every row/column)
	// just looks like scaling, not turning - there's no depth cue. Real perspective makes
	// the edge farther from the hinge shrink in the *other* axis too (a keystone/trapezoid
	// shape), which is what actually reads as "this is rotating in 3D, not just squashing."
	// This draws `shown` in thin strips along the hinge axis, applying that keystone falloff.
	static void PerspectiveFlip(Draw& w, Rect dest, const Image& shown, double squeeze, bool horizontal, bool hinge_far)
	{
		double theta = acos(clamp(squeeze, 0.0, 1.0));
		double s     = sin(theta);
		Size   isz   = shown.GetSize();
		int    strips = 28;
		for(int i = 0; i < strips; i++) {
			double u0 = double(i) / strips, u1 = double(i + 1) / strips;
			double um      = (u0 + u1) / 2;
			double localu  = hinge_far ? (1 - um) : um;              // 0 at hinge, 1 at far edge
			double persp   = 1 - localu * s * 0.55;                  // keystone falloff
			double screenu = hinge_far ? (1 - (1 - um) * squeeze) : (um * squeeze);
			if(horizontal) {
				Rect src(int(u0 * isz.cx), 0, int(u1 * isz.cx) + 1, isz.cy);
				int  stripw = max(1, dest.Width() / strips + 1);
				int  sx = dest.left + int(screenu * dest.Width()) - stripw / 2;
				int  sh = max(1, int(dest.Height() * persp));
				int  sy = dest.top + (dest.Height() - sh) / 2;
				w.DrawImage(RectC(sx, sy, stripw, sh), shown, src);
			} else {
				Rect src(0, int(u0 * isz.cy), isz.cx, int(u1 * isz.cy) + 1);
				int  striph = max(1, dest.Height() / strips + 1);
				int  sy = dest.top + int(screenu * dest.Height()) - striph / 2;
				int  sw = max(1, int(dest.Width() * persp));
				int  sx = dest.left + (dest.Width() - sw) / 2;
				w.DrawImage(RectC(sx, sy, sw, striph), shown, src);
			}
		}
	}

	// Same idea as PerspectiveFlip but folding out of one corner in both axes at once - a
	// third, visually distinct kind of 3D turn (grid of small tiles instead of strips).
	static void PerspectiveCornerFlip(Draw& w, Rect dest, const Image& shown, double squeeze, int corner)
	{
		double theta = acos(clamp(squeeze, 0.0, 1.0));
		double s     = sin(theta);
		Size   isz   = shown.GetSize();
		bool   farx  = corner & 1, fary = corner & 2;
		int    n     = 10;
		for(int j = 0; j < n; j++) {
			double v0 = double(j) / n, v1 = double(j + 1) / n, vm = (v0 + v1) / 2;
			double localv  = fary ? (1 - vm) : vm;
			double screenv = fary ? (1 - (1 - vm) * squeeze) : (vm * squeeze);
			for(int i = 0; i < n; i++) {
				double u0 = double(i) / n, u1 = double(i + 1) / n, um = (u0 + u1) / 2;
				double localu  = farx ? (1 - um) : um;
				double screenu = farx ? (1 - (1 - um) * squeeze) : (um * squeeze);
				double persp   = 1 - (localu + localv) / 2 * s * 0.55;
				Rect src(int(u0 * isz.cx), int(v0 * isz.cy), int(u1 * isz.cx) + 1, int(v1 * isz.cy) + 1);
				int  tw = max(1, int(dest.Width()  / n * persp) + 1);
				int  th = max(1, int(dest.Height() / n * persp) + 1);
				int  cx = dest.left + int(screenu * dest.Width());
				int  cy = dest.top  + int(screenv * dest.Height());
				w.DrawImage(RectC(cx - tw / 2, cy - th / 2, tw, th), shown, src);
			}
		}
	}

	void Tick()
	{
		anim = min(1.0, anim + 0.045);
		Refresh();
		if(anim < 1.0)
			tick.Set(16, THISBACK(Tick));
	}

public:
	void SetCard(int index)
	{
		card_index = index;
		anim       = 0;
		effect     = Random(EFFECT_COUNT);
		dir        = Random(4);
		tick.KillSet(16, THISBACK(Tick));
	}

	virtual void Paint(Draw& w)
	{
		Size  sz   = GetSize();
		Color felt = Color(20, 90, 50);
		w.DrawRect(sz, felt);

		Image card = LoadCard(card_index);
		if(card.IsEmpty())
			return;

		double e    = EaseOutCubic(anim);
		Size   isz  = card.GetSize();
		int    h    = min(sz.cy - 20, 190);
		int    wd   = h * isz.cx / isz.cy;
		Rect   dest = RectC((sz.cx - wd) / 2, (sz.cy - h) / 2, wd, h);

		switch(effect) {
		case FADE_MOVE: {
			static const Point offs[] = { Point(-1, 0), Point(1, 0), Point(0, -1), Point(0, 1) };
			Rect d = dest.Offseted(offs[dir] * int((1 - e) * 50));
			PaintShadow(w, sz, d, e);
			w.DrawImage(d, Faded(card, e));
			break;
		}
		case ROTATE_2D: {
			double angle = (dir & 1 ? 1 : -1) * (1 - e) * (M_PI / 2.2);
			double scale = 0.5 + 0.5 * e;
			PaintShadow(w, sz, dest, e);
			DrawPainter dp(w, sz);
			dp.Clear(RGBAZero());
			Point c = dest.CenterPoint();
			// Xform2D's operator* composes "apply left side first, then right side" - so
			// this reads, in order: center the image at the origin, scale it, rotate it,
			// then move it to the destination center.
			Xform2D t = Xform2D::Translation(-isz.cx / 2.0, -isz.cy / 2.0)
			          * Xform2D::Scale(scale * dest.Width() / (double)isz.cx, scale * dest.Height() / (double)isz.cy)
			          * Xform2D::Rotation(angle)
			          * Xform2D::Translation(c.x, c.y);
			dp.Rectangle(0, 0, sz.cx, sz.cy).Fill(Faded(card, e), t);
			break;
		}
		case FLIP_H: { // 3D-style turn around the vertical axis, hinged left or right
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			PerspectiveFlip(w, dest, shown, squeeze, true, dir & 1);
			break;
		}
		case FLIP_V: { // 3D-style turn around the horizontal axis, hinged top or bottom
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			PerspectiveFlip(w, dest, shown, squeeze, false, dir & 1);
			break;
		}
		case FLIP_DIAG: { // 3D-style turn folding out of a random corner (both axes at once)
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			PerspectiveCornerFlip(w, dest, shown, squeeze, dir);
			break;
		}
		case MAGIC_CARPET: { // wavy unfurl, ripple settling out as the card lands
			PaintShadow(w, sz, dest, e);
			int strips = 20;
			for(int i = 0; i < strips; i++) {
				double v0 = double(i) / strips, v1 = double(i + 1) / strips;
				Rect   src(0, int(v0 * isz.cy), isz.cx, int(v1 * isz.cy) + 1);
				int    sy = dest.top + int(v0 * dest.Height());
				int    sh = max(1, dest.Height() / strips + 1);
				double wobble = sin(v0 * M_PI * 4 + anim * 14 + dir) * (1 - e) * 22;
				Rect   d(dest.left + int(wobble), sy, dest.left + int(wobble) + dest.Width(), sy + sh);
				w.DrawImage(d, Faded(card, e), src);
			}
			break;
		}
		}
	}
};

GUI_APP_MAIN
{
	String file = ConfigFile("CardExample.form");
	if(CommandLine().GetCount())
		file = CommandLine()[0];
	FormWindow form;
	if(!form.Load(file)) {
		Exclamation("Could not load form: " + DeQtf(file));
		return;
	}
	form.Layout("Default");
	Form& f = form.GetForm();

	// The .form's "CardLabel" is kept only to reserve/zoom the layout rectangle;
	// the actual card is drawn by CardArt, added on top, and the label is hidden.
	Ctrl *placeholder = f.GetCtrl("CardLabel");
	Rect  card_rect   = placeholder ? placeholder->GetRect() : RectC(20, 20, 400, 140);
	if(placeholder)
		placeholder->Hide();

	CardArt art;
	f.Add(art);
	art.SetRect(card_rect);

	f.SignalHandler << [&art](const String&, const String& op, const String& action) {
		if(op == "OnAction" && action == "GiveCard")
			art.SetCard(Random(52));
	};
	form.Run();
}
