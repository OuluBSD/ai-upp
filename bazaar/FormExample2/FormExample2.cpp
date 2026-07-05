#include <Core/Core.h>
using namespace Upp;

#include <Form/Form.hpp>
#include <Ctrl/Easing/Easing.h>
#include <Ctrl/Xform3D/Xform3D.h>

// Preview app for the CtrlCore Easing/Transforms tracks (see plan/CompositeEasingNetwork):
// drives an ease-out Tween (uppsrc/Ctrl/Easing) animating a real themed, alpha-channel card
// asset (share/imgs/cards/default/, the same theme directory game/TexasHoldem's GameTable
// uses) in via one of several distinct transition styles - fade+move, 2D rotation, three
// different flip-style turns (each showing the card back first, then revealing the face
// partway through, like an actual card flip, now using genuine perspective rotation via
// uppsrc/Ctrl/Xform3D instead of a hand-rolled keystone approximation), and a wavy "magic
// carpet" unfurl - picked at random, from a random direction, on every deal.
class CardArt : public Ctrl {
	typedef CardArt CLASSNAME;

	enum Effect { FADE_MOVE, ROTATE_2D, FLIP_H, FLIP_V, FLIP_DIAG, MAGIC_CARPET, EFFECT_COUNT };

	int    card_index = -1;  // -1 = card back, 0..51 = suit*13 + rank
	double progress   = 1.0; // eased 0 = just dealt, 1 = settled (already curve-applied by tween)
	int    effect     = FADE_MOVE;
	int    dir        = 0;   // 0..3, meaning depends on the effect
	Tween  tween;

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

	// Turns FlipFace()'s 0..1 squeeze into the [0, pi/2] angle Xform3D::Set expects
	// (0 = fully face-on, pi/2 = fully edge-on) - the same acos() relationship the
	// old hand-rolled PerspectiveFlip/PerspectiveCornerFlip used for their keystone
	// falloff, now driving a genuine 3D perspective rotation instead.
	static double SqueezeToAngle(double squeeze)
	{
		return acos(clamp(squeeze, 0.0, 1.0));
	}

public:
	void SetCard(int index)
	{
		card_index = index;
		effect     = Random(EFFECT_COUNT);
		dir        = Random(4);
		progress   = 0;
		// ~350ms total, matching the old anim += 0.045 per 16ms tick's overall feel.
		tween.Start(350, EaseOutCubic, [this](double e) { progress = e; Refresh(); });
	}

	virtual void Paint(Draw& w)
	{
		Size  sz   = GetSize();
		Color felt = Color(20, 90, 50);
		w.DrawRect(sz, felt);

		Image card = LoadCard(card_index);
		if(card.IsEmpty())
			return;

		double e    = progress; // already curve-applied by the Tween
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
		case FLIP_H: { // genuine 3D turn around the vertical axis, hinged left or right
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			double angle = SqueezeToAngle(squeeze);
			Xform3D xf;
			// angle_y = vertical hinge (left/right turn); sign picks which side leads.
			xf.Set(0.0, dir & 1 ? -angle : angle, dest.GetSize());
			DrawWarped3D(w, shown, xf, 10, dest.CenterPoint());
			break;
		}
		case FLIP_V: { // genuine 3D turn around the horizontal axis, hinged top or bottom
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			double angle = SqueezeToAngle(squeeze);
			Xform3D xf;
			// angle_x = horizontal hinge (top/bottom turn); sign picks which side leads.
			xf.Set(dir & 1 ? -angle : angle, 0.0, dest.GetSize());
			DrawWarped3D(w, shown, xf, 10, dest.CenterPoint());
			break;
		}
		case FLIP_DIAG: { // genuine 3D turn folding out of a random corner (both axes at once)
			double squeeze;
			Image  shown = Faded(FlipFace(e, squeeze), max(0.5, e));
			PaintShadow(w, sz, dest, e);
			double angle = SqueezeToAngle(squeeze);
			Xform3D xf;
			// Both hinges at once, corner picked by dir's two bits (same "which corner"
			// encoding the old PerspectiveCornerFlip used).
			xf.Set(dir & 1 ? -angle : angle, dir & 2 ? -angle : angle, dest.GetSize());
			DrawWarped3D(w, shown, xf, 10, dest.CenterPoint());
			break;
		}
		case MAGIC_CARPET: { // wavy unfurl, ripple settling out as the card lands
			PaintShadow(w, sz, dest, e);
			int   strips = 20;
			Image faded  = Faded(card, e);

			// wobble()'s amplitude is capped at (1-e)*22 <= 22, so +/-22px plus a
			// little slack is enough headroom for the offscreen buffer in any
			// direction, same "bound just what this warp needs" idiom as
			// DrawWarped3D (Xform3D.cpp).
			int  margin = 24;
			int  bx0    = dest.left - margin;
			int  bx1    = dest.left + dest.Width() + margin;
			int  by0    = dest.top;
			int  by1    = dest.top + dest.Height() + margin;
			Size bufsz(max(1, bx1 - bx0), max(1, by1 - by0));

			// MODE_NOAA (no antialiasing) from the start: independently-antialiased
			// adjacent strips sharing an edge in a shared transparent buffer don't
			// sum to full opacity at the shared boundary (the seam bug Transforms/0005
			// already had to fix once) - build hard-edged instead of reproducing it.
			ImagePainter ip(bufsz, MODE_NOAA);
			ip.Clear(RGBAZero());

			for(int i = 0; i < strips; i++) {
				double v0 = double(i) / strips, v1 = double(i + 1) / strips;
				Rect   src(0, int(v0 * isz.cy), isz.cx, int(v1 * isz.cy) + 1);
				int    sy = dest.top + int(v0 * dest.Height());
				int    sh = max(1, dest.Height() / strips + 1);
				// Evaluated at both edges (not just v0) so that strip i's bottom
				// edge (v1) and strip i+1's top edge (same v value) land on the
				// exact same wobble - adjacent strips then share an exact boundary
				// position instead of stepping.
				double wobble0 = sin(v0 * M_PI * 4 + e * 14 + dir) * (1 - e) * 22;
				double wobble1 = sin(v1 * M_PI * 4 + e * 14 + dir) * (1 - e) * 22;

				Pointf p00(dest.left + wobble0 - bx0, sy - by0);
				Pointf p10(dest.left + wobble0 + dest.Width() - bx0, sy - by0);
				Pointf p01(dest.left + wobble1 - bx0, sy + sh - by0);
				Pointf p11(dest.left + wobble1 + dest.Width() - bx0, sy + sh - by0);

				// Real per-strip affine warp (3-point correspondence: top-left, top-right,
				// bottom-left) instead of an axis-aligned blit, same idiom as DrawWarped3D's
				// per-cell Xform2D::Map. The 4th corner (bottom-right, p11) isn't part of
				// that basis and won't generally land exactly under a pure affine map - an
				// accepted limitation, same as DrawWarped3D - but the clip path below still
				// uses the true 4-corner quad (including p11), so adjacent strips' shared
				// edges line up exactly.
				Xform2D t = Xform2D::Map(Pointf(src.left, src.top), Pointf(src.right, src.top),
				                         Pointf(src.left, src.bottom),
				                         p00, p10, p01);

				ip.Move(p00).Line(p10).Line(p11).Line(p01).Close();
				ip.Fill(faded, t);
			}

			w.DrawImage(bx0, by0, ip.GetResult());
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
