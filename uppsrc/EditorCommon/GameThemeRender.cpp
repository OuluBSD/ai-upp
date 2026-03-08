#include <EditorCommon/GameThemeRender.h>
#include <GameCommon/Poker/TexasRenderer.h>
#include <GameCommon/Poker/TableLayoutProfile.h>
#include <Painter/Painter.h>

namespace Upp {

static Image LoadThemeAssetImage(const String& project_name, const String& asset)
{
	String p = ResolveThemeAssetPath(project_name, asset);
	if (p.IsEmpty() || !FileExists(p))
		return Image();
	return StreamRaster::LoadFileAny(p);
}

static Rect ClipRectToImage(const Rect& r, Size sz)
{
	Rect c = r;
	c.left = max(0, min(c.left, sz.cx));
	c.top = max(0, min(c.top, sz.cy));
	c.right = max(c.left, min(c.right, sz.cx));
	c.bottom = max(c.top, min(c.bottom, sz.cy));
	return c;
}

static Image RotatePatch90(const Image& img, int quarters)
{
	int q = ((quarters % 4) + 4) % 4;
	Image out = img;
	while (q-- > 0) {
		Size s = out.GetSize();
		ImageBuffer src(out);
		ImageBuffer dst(s.cy, s.cx);
		for (int y = 0; y < s.cy; y++)
			for (int x = 0; x < s.cx; x++)
				dst[x][s.cy - 1 - y] = src[y][x];
		out = Image(dst);
	}
	return out;
}

static Image RotatePatchAny(const Image& img, int quarters, int angle_deg)
{
	if (img.IsEmpty())
		return img;
	if (angle_deg != 0)
		return Rotate(img, angle_deg);
	return RotatePatch90(img, quarters);
}

static Image ApplyCardRotations(const Image& src,
                                const int board_quarters[5], const int hero_quarters[2],
                                const int board_angle_deg[5], const int hero_angle_deg[2])
{
	if (src.IsEmpty())
		return src;
	Size sz = src.GetSize();
	Image out = src;
	auto rotate_at = [&](Rect rr, int quarters, int angle_deg) {
		if (quarters == 0 && angle_deg == 0)
			return;
		Rect r = ClipRectToImage(rr, sz);
		if (r.IsEmpty())
			return;
		Image patch = Crop(out, r);
		if (patch.IsEmpty())
			return;
		Image rot = RotatePatchAny(patch, quarters, angle_deg);
		rot = Rescale(rot, r.GetWidth(), r.GetHeight());
		ImageBuffer ib(out.GetSize());
		BufferPainter bp(ib);
		bp.Begin();
		bp.DrawImage(0, 0, out);
		bp.DrawImage(r.left, r.top, rot);
		bp.End();
		out = Image(ib);
	};
	for (int i = 0; i < 5; i++)
		rotate_at(TexasTableLayout::BoardCardRect(sz, i), board_quarters[i], board_angle_deg[i]);
	for (int i = 0; i < 2; i++)
		rotate_at(TexasTableLayout::HeroCardRect(sz, i), hero_quarters[i], hero_angle_deg[i]);
	return out;
}

void ThemeApplyCardTheme(const Vector<ThemeObject>& objs, const String& theme_profile)
{
	String card_theme = "default_800x480";
	String card_back = "flipside.png";
	for (const ThemeObject& o : objs) {
		if (!ThemeObjectVisibleForProfile(o, theme_profile) || ToLower(TrimBoth(o.type)) != "card-theme")
			continue;
		card_theme = ThemeGetPropString(o.props, "theme", card_theme);
		card_back = ThemeGetPropString(o.props, "back_image", card_back);
	}
	TexasRenderer::SetCardTheme(card_theme, card_back);
}

Image ThemeOverlayObjects(const Image& src, const Vector<ThemeObject>& objs, const GameState* state,
                          const String& project_name, const String& theme_profile)
{
	if (src.IsEmpty())
		return src;
	int board_quarters[5] = {0, 0, 0, 0, 0};
	int hero_quarters[2] = {0, 0};
	int board_angle_deg[5] = {0, 0, 0, 0, 0};
	int hero_angle_deg[2] = {0, 0};
	ImageBuffer ib(src.GetSize());
	BufferPainter bp(ib);
	bp.Begin();
	for (const ThemeObject& o : objs) {
		if (!ThemeObjectVisibleForProfile(o, theme_profile))
			continue;
		String t = ToLower(TrimBoth(o.type));
		if (t != "background")
			continue;
		Image bg = LoadThemeAssetImage(project_name, ThemeGetPropString(o.props, "asset"));
		if (!bg.IsEmpty())
			bp.DrawImage(o.rect.left, o.rect.top, o.rect.GetWidth(), o.rect.GetHeight(), bg);
	}
	bp.DrawImage(0, 0, src);
	for (const ThemeObject& o : objs) {
		if (!ThemeObjectVisibleForProfile(o, theme_profile))
			continue;
		Color c = ThemeParseColor(o.color);
		String t = ToLower(TrimBoth(o.type));
		if (t == "text") {
			Rect r = o.rect;
			int fs = max(8, ThemeGetPropInt(o.props, "font_size", 15));
			Font f = StdFont(fs);
			String face = ThemeGetPropString(o.props, "font_face");
			if (!face.IsEmpty())
				f = f.FaceName(face);
			if (ThemeGetPropBool(o.props, "bold", true))
				f = f.Bold();
			if (ThemeGetPropBool(o.props, "italic", false))
				f = f.Italic();
			String txt = o.text.IsEmpty() ? o.name : o.text;
			bp.DrawText(r.left + 3, r.top + 3, txt, f, c);
		}
		else if (t == "image") {
			Rect r = o.rect;
			Image img = LoadThemeAssetImage(project_name, ThemeGetPropString(o.props, "asset"));
			if (!img.IsEmpty())
				bp.DrawImage(r.left, r.top, r.GetWidth(), r.GetHeight(), img);
			else {
				bp.DrawRect(r, c);
				bp.DrawLine(r.left, r.top, r.right, r.bottom, 1, c);
				bp.DrawLine(r.left, r.bottom, r.right, r.top, 1, c);
				bp.DrawText(r.left + 3, r.top + 3, o.name, StdFont(11), c);
			}
		}
		else if (t == "table") {
			Rect r = o.rect;
			bp.DrawRect(r, c);
			Color edge = Color(min(255, c.GetR() + 70), min(255, c.GetG() + 70), min(255, c.GetB() + 70));
			bp.DrawRect(r.left, r.top, r.GetWidth(), 2, edge);
			bp.DrawRect(r.left, r.bottom - 2, r.GetWidth(), 2, edge);
			bp.DrawRect(r.left, r.top, 2, r.GetHeight(), edge);
			bp.DrawRect(r.right - 2, r.top, 2, r.GetHeight(), edge);
		}
		else if (t == "tabs") {
			Rect r = o.rect;
			int n = max(1, ThemeGetPropInt(o.props, "tab_count", 4));
			int active = minmax(ThemeGetPropInt(o.props, "active_index", 0), 0, n - 1);
			bp.DrawRect(r, Color(40, 40, 40));
			int tw = max(1, r.GetWidth() / n);
			for (int i = 0; i < n; i++) {
				Rect tr(r.left + i * tw, r.top, (i == n - 1) ? r.right : (r.left + (i + 1) * tw), r.bottom);
				Color tc = (i == active) ? c : Color(85, 85, 85);
				bp.DrawRect(tr, tc);
				String label = Format("T%d", i + 1);
				bp.DrawText(tr.left + 6, tr.top + max(2, (tr.GetHeight() - 15) / 2), label, StdFont(12).Bold(), White());
			}
		}
		else if (t == "doc-edit") {
			Rect r = o.rect;
			int lines = max(1, ThemeGetPropInt(o.props, "lines", 6));
			bp.DrawRect(r, White());
			bp.DrawRect(r.left, r.top, r.GetWidth(), 1, Color(120, 120, 120));
			bp.DrawRect(r.left, r.bottom - 1, r.GetWidth(), 1, Color(120, 120, 120));
			bp.DrawRect(r.left, r.top, 1, r.GetHeight(), Color(120, 120, 120));
			bp.DrawRect(r.right - 1, r.top, 1, r.GetHeight(), Color(120, 120, 120));
			for (int i = 0; i < lines; i++) {
				int y = r.top + 6 + i * max(12, (r.GetHeight() - 10) / lines);
				if (y + 2 >= r.bottom)
					break;
				bp.DrawRect(r.left + 6, y, max(10, r.GetWidth() - 12), 1, Color(170, 170, 170));
			}
		}
		else if (t == "chips") {
			Rect r = o.rect;
			int pot = state ? state->pot : 0;
			int chip_value = max(1, ThemeGetPropInt(o.props, "chip_value", 25));
			int max_chips = max(1, ThemeGetPropInt(o.props, "max_chips", 20));
			int chips = min(max_chips, max(1, pot / chip_value));
			int rows = max(1, ThemeGetPropInt(o.props, "rows", 2));
			int cols = max(1, (chips + rows - 1) / rows);
			int cw = max(6, r.GetWidth() / max(1, cols));
			int ch = max(6, r.GetHeight() / max(1, rows));
			int idx = 0;
			for (int yy = 0; yy < rows && idx < chips; yy++) {
				for (int xx = 0; xx < cols && idx < chips; xx++, idx++) {
					int cx = r.left + xx * cw + cw / 2;
					int cy = r.top + yy * ch + ch / 2;
					int rad = max(3, min(cw, ch) / 2 - 1);
					bp.DrawEllipse(cx - rad, cy - rad, 2 * rad, 2 * rad, c, 1, White());
				}
			}
			bp.DrawText(r.left + 2, r.top + 2, Format("POT %d", pot), StdFont(11).Bold(), White());
		}
		else if (t == "card-rotate") {
			int q = ThemeGetPropInt(o.props, "quarters", 0);
			int angle = ThemeGetPropInt(o.props, "angle_deg", 0);
			String target = ToLower(ThemeGetPropString(o.props, "target", "all"));
			if (target == "all" || target == "board") {
				for (int i = 0; i < 5; i++) {
					board_quarters[i] = q;
					board_angle_deg[i] = angle;
				}
			}
			if (target == "all" || target == "hero" || target == "hand") {
				for (int i = 0; i < 2; i++) {
					hero_quarters[i] = q;
					hero_angle_deg[i] = angle;
				}
			}
			hero_quarters[0] = ThemeGetPropInt(o.props, "hero1_quarters", hero_quarters[0]);
			hero_quarters[1] = ThemeGetPropInt(o.props, "hero2_quarters", hero_quarters[1]);
			hero_angle_deg[0] = ThemeGetPropInt(o.props, "hero1_angle_deg", hero_angle_deg[0]);
			hero_angle_deg[1] = ThemeGetPropInt(o.props, "hero2_angle_deg", hero_angle_deg[1]);
			board_quarters[0] = ThemeGetPropInt(o.props, "flop1_quarters", board_quarters[0]);
			board_quarters[1] = ThemeGetPropInt(o.props, "flop2_quarters", board_quarters[1]);
			board_quarters[2] = ThemeGetPropInt(o.props, "flop3_quarters", board_quarters[2]);
			board_quarters[3] = ThemeGetPropInt(o.props, "turn_quarters", board_quarters[3]);
			board_quarters[4] = ThemeGetPropInt(o.props, "river_quarters", board_quarters[4]);
			board_angle_deg[0] = ThemeGetPropInt(o.props, "flop1_angle_deg", board_angle_deg[0]);
			board_angle_deg[1] = ThemeGetPropInt(o.props, "flop2_angle_deg", board_angle_deg[1]);
			board_angle_deg[2] = ThemeGetPropInt(o.props, "flop3_angle_deg", board_angle_deg[2]);
			board_angle_deg[3] = ThemeGetPropInt(o.props, "turn_angle_deg", board_angle_deg[3]);
			board_angle_deg[4] = ThemeGetPropInt(o.props, "river_angle_deg", board_angle_deg[4]);
		}
	}
	bp.End();
	return ApplyCardRotations(Image(ib), board_quarters, hero_quarters, board_angle_deg, hero_angle_deg);
}

}
