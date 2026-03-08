#include <EditorCommon/Recognition.h>
#include <EditorCommon/GpuPreprocess.h>
#include <GameRules/GameDefs.h>
#include <plugin/png/png.h>

#include "StrategyBridge/StrategyBridge.h"
#ifdef PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace Upp {

Image CropRectSafe(const Image& src, const Rect& r) {
	if (src.IsEmpty() || r.IsEmpty()) return Image();
	Rect rr = r;
	rr.Intersect(src.GetSize());
	if (rr.IsEmpty()) return Image();
	return Crop(src, rr);
}

static inline byte GetGrayscale(byte r, byte g, byte b) {
	return (byte)((r * 77 + g * 150 + b * 29) >> 8);
}

static void FillVolumeFromByteMat(const ByteMat& src, ConvNet::Volume& x, int out_w, int out_h) {
	double sx = (double)src.cols / out_w; double sy = (double)src.rows / out_h;
	int out_d = x.GetDepth();
	for (int y = 0; y < out_h; y++) {
		for (int x_idx = 0; x_idx < out_w; x_idx++) {
			double fx = x_idx * sx; double fy = y * sy;
			int ix = (int)fx; int iy = (int)fy;
			double dx = fx - ix; double dy = fy - iy;
			
			for (int d = 0; d < out_d; d++) {
				auto get_p = [&](int px, int py) {
					px = min(max(px, 0), src.cols - 1); py = min(max(py, 0), src.rows - 1);
					int c = (src.channels == 3) ? d : 0; 
					return src.data[(py * src.cols + px) * src.channels + c];
				};
				double val = get_p(ix, iy) * (1-dx)*(1-dy) + get_p(ix+1, iy) * dx*(1-dy) +
				             get_p(ix, iy+1) * (1-dx)*dy + get_p(ix+1, iy+1) * dx*dy;
				x.Set(x_idx, y, d, val / 255.0 - 0.5);
			}
		}
	}
}

static void FillVolumeFromImage(const Image& patch, ConvNet::Volume& x, int out_w, int out_h) {
	double sx = (double)patch.GetWidth() / out_w; double sy = (double)patch.GetHeight() / out_h;
	int out_d = x.GetDepth();
	for (int y = 0; y < out_h; y++) {
		for (int x_idx = 0; x_idx < out_w; x_idx++) {
			double fx = x_idx * sx; double fy = y * sy;
			int ix = (int)fx; int iy = (int)fy;
			double dx = fx - ix; double dy = fy - iy;
			auto get_p = [&](int px, int py) {
				px = min(max(px, 0), patch.GetWidth() - 1); py = min(max(py, 0), patch.GetHeight() - 1);
				RGBA c = patch[py][px];
				return GetGrayscale(c.r, c.g, c.b);
			};
			double val = get_p(ix, iy) * (1-dx)*(1-dy) + get_p(ix+1, iy) * dx*(1-dy) +
			             get_p(ix, iy+1) * (1-dx)*dy + get_p(ix+1, iy+1) * dx*dy;
			double norm = val / 255.0 - 0.5;
			for (int d = 0; d < out_d; d++)
				x.Set(x_idx, y, d, norm);
		}
	}
}

OCRModule::OCRModule() {
#ifdef PLATFORM_WIN32
	hTess = LoadLibrary("libtesseract-5.dll");
#else
	hTess = dlopen("libtesseract.so.5", RTLD_LAZY);
	if (!hTess) hTess = dlopen("libtesseract.so.4", RTLD_LAZY);
	if (!hTess) hTess = dlopen("libtesseract.so", RTLD_LAZY);
#endif
	if (hTess) {
#ifdef PLATFORM_WIN32
#define LOAD_TESS(name) *(void**)(&name) = (void*)GetProcAddress((HMODULE)hTess, #name)
#else
#define LOAD_TESS(name) *(void**)(&name) = (void*)dlsym(hTess, #name)
#endif
		LOAD_TESS(TessBaseAPICreate);
		LOAD_TESS(TessBaseAPIInit3);
		LOAD_TESS(TessBaseAPISetImage);
		LOAD_TESS(TessBaseAPIGetUTF8Text);
		LOAD_TESS(TessBaseAPIDelete);
		LOAD_TESS(TessDeleteText);
#undef LOAD_TESS
		
		if (TessBaseAPICreate && TessBaseAPIInit3) {
			api = TessBaseAPICreate();
			String datapath = GetExeDirFile("tessdata");
			int rc = TessBaseAPIInit3(api, datapath, "eng");
			if (rc == 0) {
				loaded = true;
				typedef void (* PFNSETPSM) (void*, int);
				PFNSETPSM setPsm = (PFNSETPSM)dlsym(hTess, "TessBaseAPISetPageSegMode");
				if (setPsm) setPsm(api, 7);
			}
		}
	}
}

OCRModule::~OCRModule() {
	if (api && TessBaseAPIDelete) TessBaseAPIDelete(api);
}

Image OCRModule::Preprocess(const Image& img) {
	if (img.IsEmpty()) return img;
	int w = img.GetWidth(), h = img.GetHeight();
	ImageBuffer ib(w, h);
	Vector<uint32> integral;
	integral.SetCount((w + 1) * (h + 1), 0);
	for (int y = 0; y < h; y++) {
		uint32 row_sum = 0;
		for (int x = 0; x < w; x++) {
			RGBA c = img[y][x];
			row_sum += GetGrayscale(c.r, c.g, c.b);
			integral[(y + 1) * (w + 1) + (x + 1)] = integral[y * (w + 1) + (x + 1)] + row_sum;
		}
	}
	const int r = 7;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int x1 = max(0, x - r), x2 = min(w - 1, x + r);
			int y1 = max(0, y - r), y2 = min(h - 1, y + r);
			int count = (x2 - x1 + 1) * (y2 - y1 + 1);
			uint32 sum = integral[(y2 + 1) * (w + 1) + (x2 + 1)] - integral[y1 * (w + 1) + (x2 + 1)] -
			             integral[(y2 + 1) * (w + 1) + x1] + integral[y1 * (w + 1) + x1];
			int mean = (int)(sum / count);
			RGBA c = img[y][x];
			int val = GetGrayscale(c.r, c.g, c.b);
			ib[y][x] = val > (mean - 15) ? White() : Black();
		}
	}
	return ib;
}

String OCRModule::ReadText(const Image& img) {
	if (!loaded || img.IsEmpty()) return "";
	Image patch = img;
	if (img.GetHeight() < 32) {
		double scale = 40.0 / img.GetHeight();
		patch = Rescale(img, (int)(img.GetWidth() * scale), 40);
	}
	Image pre = Preprocess(patch);
	int white_pixels = 0;
	const RGBA* pp = pre.Begin();
	for (int i = 0; i < pre.GetLength(); i++) if (pp[i].r > 128) white_pixels++;
	if (white_pixels < 5 || white_pixels > pre.GetLength() * 0.95) return "";
	TessBaseAPISetImage(api, (const unsigned char*)~pre, pre.GetWidth(), pre.GetHeight(), 4, pre.GetWidth() * 4);
	char* outText = TessBaseAPIGetUTF8Text(api);
	String result(outText);
	TessDeleteText(outText);
	return TrimBoth(result);
}

String OCRModule::ReadText(const ByteMat& gray) {
	if (!loaded || gray.IsEmpty()) return "";
	int w = gray.cols, h = gray.rows;
	Vector<int64> integral;
	integral.SetCount((w + 1) * (h + 1), 0);
	for (int y = 0; y < h; y++) {
		int64 row_sum = 0;
		for (int x = 0; x < w; x++) {
			row_sum += gray.data[y * w + x];
			integral[(y + 1) * (w + 1) + (x + 1)] = integral[y * (w + 1) + (x + 1)] + row_sum;
		}
	}
	ImageBuffer ib(w, h);
	const int r = 7;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int x1 = max(0, x - r), x2 = min(w - 1, x + r);
			int y1 = max(0, y - r), y2 = min(h - 1, y + r);
			int count = (x2 - x1 + 1) * (y2 - y1 + 1);
			int64 sum = integral[(y2 + 1) * (w + 1) + (x2 + 1)] - integral[y1 * (w + 1) + (x2 + 1)] -
			            integral[(y2 + 1) * (w + 1) + x1] + integral[y1 * (w + 1) + x1];
			int mean = (int)(sum / count);
			int val = gray.data[y * w + x];
			ib[y][x] = val > (mean - 10) ? White() : Black();
		}
	}
	Image pre = ib;
	TessBaseAPISetImage(api, (const unsigned char*)~pre, w, h, 4, w * 4);
	char* outText = TessBaseAPIGetUTF8Text(api);
	String result(outText);
	TessDeleteText(outText);
	return TrimBoth(result);
}

CardClassifier::CardClassifier() {}

bool CardClassifier::Load(const String& model_dir, const String& platform) {
	auto load_tag = [&](const String& tag, ConvNet::Session& ses, bool& loaded, int& w, int& h, int& d, Vector<String>& classes) {
		String base = AppendFileName(model_dir, tag);
		if (!platform.IsEmpty()) {
			String p_base = AppendFileName(model_dir, platform + "_" + tag);
			if (FileExists(p_base + ".model.bin")) base = p_base;
		}
		String bin = base + ".model.bin";
		if (!FileExists(bin)) bin = base + ".bin";
		String meta_path = base + ".metadata.json";
		if (FileExists(meta_path)) {
			Value m = ParseJSON(LoadFile(meta_path));
			if (!IsNull(m)) {
				w = m["width"]; h = m["height"]; d = m["depth"];
				const Value& cls = m["classes"];
				classes.Clear();
				for (int i = 0; i < cls.GetCount(); i++) classes.Add(cls[i]);
			}
		}
		if (FileExists(bin)) {
			FileIn fi(bin);
			ses.Serialize(fi);
			if (ses.GetLayerCount() > 0) {
				if (w <= 0) { w = 20; h = 32; d = 3; }
				loaded = true; return true;
			}
		}
		return false;
	};
	unified_loaded = load_tag("unified", unified_ses, unified_loaded, unified_w, unified_h, unified_d, unified_classes);
	suit_loaded = load_tag("suit", suit_ses, suit_loaded, suit_w, suit_h, suit_d, suit_classes);
	rank_loaded = load_tag("rank", rank_ses, rank_loaded, rank_w, rank_h, rank_d, rank_classes);
	return IsLoaded();
}

int CardClassifier::ClassifyUnified(const Image& img) {
	if (!unified_loaded) return -1;
	ConvNet::Volume x(unified_w, unified_h, unified_d, 0.0);
	FillVolumeFromImage(img, x, unified_w, unified_h);
	ConvNet::Net& net = unified_ses.GetNetwork();
	net.Forward(x, false);
	return net.GetPrediction();
}

int CardClassifier::ClassifyUnified(const ByteMat& gray) {
	if (!unified_loaded) return -1;
	ConvNet::Volume x(unified_w, unified_h, unified_d, 0.0);
	FillVolumeFromByteMat(gray, x, unified_w, unified_h);
	ConvNet::Net& net = unified_ses.GetNetwork();
	net.Forward(x, false);
	return net.GetPrediction();
}

RecognitionEngine::RecognitionEngine() {
	orb.InitDefault();
	card_clf.Load(AppendFileName(GetExeDirFile("data"), "convnet_models"));
	InitStrategy(eval_pimpl, strategy_pimpl);
}

RecognitionEngine::~RecognitionEngine() {
	CleanupStrategy(eval_pimpl, strategy_pimpl);
}

void RecognitionEngine::SetAnchor(const Image& img) {
	if (img.IsEmpty()) return;
	anchor_img = img;
	orb.SetInput(img);
	orb.TrainPattern();
	anchor_trained = true;
}

GameState RecognitionEngine::Process(const Image& frame) {
	GameState state;
	Size frame_size = frame.GetSize();
	state.pokerth_rect = Rect(frame_size);
	state.found = true;
	if (anchor_trained) {
		orb.SetInput(frame);
		orb.Process();
		if (orb.GetLastGoodMatches() >= 4) {
			const Vector<Pointf>& corners = orb.GetLastCorners();
			float minx = corners[0].x, maxx = corners[0].x, miny = corners[0].y, maxy = corners[0].y;
			for (int i = 1; i < 4; i++) {
				minx = min(minx, (float)corners[i].x); maxx = max(maxx, (float)corners[i].x);
				miny = min(miny, (float)corners[i].y); maxy = max(maxy, (float)corners[i].y);
			}
			state.pokerth_rect = Rect((int)minx, (int)miny, (int)maxx, (int)maxy);
		}
	}
	frame_size = state.pokerth_rect.GetSize();
	Vector<String> platforms = rule_mgr.GetPlatformNames();
	String best_p = rule_mgr.GetActivePlatform();
	double min_diff = 1e18;
	for (const String& p_name : platforms) {
		Size base = rule_mgr.GetBaseSize(p_name);
		if (base.cx == 0 || base.cy == 0) continue;
		double sx = (double)frame_size.cx / base.cx; double sy = (double)frame_size.cy / base.cy;
		double diff = abs(sx - sy) + abs(sx - round(sx));
		if (frame_size.cx == base.cx && frame_size.cy == base.cy) diff = -10.0;
		if (diff < min_diff) { min_diff = diff; best_p = p_name; }
	}
	rule_mgr.SetActivePlatform(best_p);
	state.active_platform = best_p;
	for(int i = 0; i < rule_mgr.GetCount(); i++) {
		const GameRule& r = rule_mgr.GetRule(i);
		Rect abs_rect = rule_mgr.GetAbsRect(i, frame_size);
		abs_rect.Offset(state.pokerth_rect.TopLeft());
		abs_rect.Intersect(frame.GetSize());
		if (abs_rect.IsEmpty()) continue;
		if (r.type == RULE_BOARD_CARD || r.type == RULE_PLAYER_CARD || r.type == RULE_HAND_CARD) {
			int card = card_clf.ClassifyUnified(CropRectSafe(frame, abs_rect));
			if (card >= 0 && card_clf.GetConfidence(card) > 0.5) {
				if (r.type == RULE_BOARD_CARD) state.community_cards.Add(card);
				else {
					int player_id = -1;
					if (r.name.StartsWith("player")) {
						int p = 6; while (p < r.name.GetCount() && IsDigit(r.name[p])) p++;
						player_id = StrInt(r.name.Mid(6, p - 6));
					}
					if (player_id >= 0) {
						while (state.players.GetCount() <= player_id) state.players.Add();
						state.players[player_id].hand.Add(card);
					}
				}
			}
		} else if (r.type == RULE_POT_TOTAL || r.type == RULE_TEXT_AREA || r.type == RULE_PLAYER_NAME || r.type == RULE_PLAYER_STACK || r.type == RULE_PLAYER_BET || r.type == RULE_PLAYER_ACTION) {
			String text = ocr.ReadText(Grayscale(CropRectSafe(frame, abs_rect)));
			if (r.type == RULE_POT_TOTAL) state.pot = StrInt(text);
			else if (r.name.StartsWith("player")) {
				int p = 6; while (p < r.name.GetCount() && IsDigit(r.name[p])) p++;
				int player_id = StrInt(r.name.Mid(6, p - 6));
				if (player_id >= 0) {
					while (state.players.GetCount() <= player_id) state.players.Add();
					if (r.type == RULE_PLAYER_NAME) state.players[player_id].name = text;
					else if (r.type == RULE_PLAYER_STACK) state.players[player_id].stack = StrInt(text);
					else if (r.type == RULE_PLAYER_BET) state.players[player_id].bet = StrInt(text);
					else if (r.type == RULE_PLAYER_ACTION) state.players[player_id].last_action = text;
				}
			}
		} else if (r.type == RULE_DEALER_CHIP || r.type == RULE_IMAGE_BOOLEAN || r.type == RULE_ACTIVE_PLAYER) {
			Image patch = CropRectSafe(frame, abs_rect);
			int active_pixels = 0;
			RGBA* px = const_cast<RGBA*>(patch.Begin());
			for (int j = 0; j < patch.GetLength(); j++) if (GetGrayscale(px[j].r, px[j].g, px[j].b) > 200) active_pixels++;
			bool active = (active_pixels > (patch.GetWidth() * patch.GetHeight() / 10));
			if (r.name == "my_turn") state.my_turn = active;
			else if (r.name.StartsWith("player")) {
				int p = 6; while (p < r.name.GetCount() && IsDigit(r.name[p])) p++;
				int player_id = StrInt(r.name.Mid(6, p - 6));
				if (player_id >= 0) {
					while (state.players.GetCount() <= player_id) state.players.Add();
					if (r.type == RULE_DEALER_CHIP) state.players[player_id].is_dealer = active;
					else if (r.type == RULE_ACTIVE_PLAYER) state.players[player_id].is_active = active;
				}
			}
		}
	}
	state.round = scene_machine.Update(state);
	hh_builder.Update(state);
	if (eval_pimpl && strategy_pimpl && (state.players.GetCount() > 0 && state.players[0].hand.GetCount() == 2)) {
		const auto& hole = state.players[0].hand;
		const auto& board = state.community_cards;
		state.hero_equity = ComputeHeroEquity(hole, board, eval_pimpl);
		if (state.my_turn) {
			Vector<byte> history; state.advice = GetStrategyAdvice(hole, board, state.pot, history, strategy_pimpl, state.advice_probs);
		}
	}
	state.found = true;
	return state;
}

GameState RecognitionEngine::ProcessGpu(GpuPreprocessEngine& gpu) {
	GameState state;
	const GpuPreprocessStats& stats = gpu.GetStats();
	Size frame_size(stats.width, stats.height);
	if (frame_size.cx <= 0) frame_size = Size(1920, 1080);
	Vector<String> platforms = rule_mgr.GetPlatformNames();
	String best_p = rule_mgr.GetActivePlatform();
	double min_diff = 1e18;
	for (const String& p_name : platforms) {
		Size base = rule_mgr.GetBaseSize(p_name);
		if (base.cx == 0 || base.cy == 0) continue;
		double sx = (double)frame_size.cx / base.cx; double sy = (double)frame_size.cy / base.cy;
		double diff = abs(sx - sy) + abs(sx - round(sx));
		if (frame_size.cx == base.cx && frame_size.cy == base.cy) diff = -10.0;
		if (diff < min_diff) { min_diff = diff; best_p = p_name; }
	}
	rule_mgr.SetActivePlatform(best_p);
	state.active_platform = best_p;
	const auto& rules = rule_mgr.GetRules();
	Vector<Rect> ocr_rects, card_rects;
	Vector<int> ocr_indices, card_indices;
	for(int i = 0; i < rules.GetCount(); i++) {
		const GameRule& r = rules[i];
		Rect abs_rect = rule_mgr.GetAbsRect(i, frame_size);
		abs_rect.Intersect(frame_size);
		if (abs_rect.IsEmpty()) continue;
		if (r.type == RULE_BOARD_CARD || r.type == RULE_PLAYER_CARD || r.type == RULE_HAND_CARD) { card_rects.Add(abs_rect); card_indices.Add(i); }
		else { ocr_rects.Add(abs_rect); ocr_indices.Add(i); }
	}
	Vector<ByteMat> area_mats, card_mats;
	bool has_areas = gpu.ReadbackBinarizedAreas(ocr_rects, 160.0f / 255.0f, area_mats);
	bool has_cards = gpu.ReadbackAreas(card_rects, card_mats);
	for(int k = 0; k < ocr_rects.GetCount(); k++) {
		int i = ocr_indices[k]; const GameRule& r = rules[i];
		if (has_areas && k < area_mats.GetCount() && !area_mats[k].IsEmpty()) {
			const ByteMat& sub = area_mats[k];
			if (r.type == RULE_POT_TOTAL || r.type == RULE_TEXT_AREA || r.type == RULE_PLAYER_BET || r.type == RULE_PLAYER_STACK) {
				String text = ocr.ReadText(sub);
				if (r.name == "pot" || r.type == RULE_POT_TOTAL)
					state.pot = hysteresis.Filter("pot", StrInt(text));
				else if (r.name.EndsWith("_bet")) {
					int p_idx = StrInt(r.name.Mid(6, r.name.Find('_', 6) - 6));
					while (state.players.GetCount() <= p_idx) state.players.Add();
					state.players[p_idx].bet = hysteresis.Filter(r.name, StrInt(text));
				}
				else if (r.name.EndsWith("_stack")) {
					int p_idx = StrInt(r.name.Mid(6, r.name.Find('_', 6) - 6));
					while (state.players.GetCount() <= p_idx) state.players.Add();
					state.players[p_idx].stack = hysteresis.Filter(r.name, StrInt(text));
				}
			}
			else if (r.type == RULE_IMAGE_BOOLEAN || r.type == RULE_ACTIVE_PLAYER) {
				int active_pixels = 0;
				for (int j = 0; j < sub.data.GetCount(); j++) if (sub.data[j] > 150) active_pixels++;
				bool active = (active_pixels > (sub.cols * sub.rows / 10));
				if (r.name == "my_turn") state.my_turn = active;
				int player_id = -1;
				if (r.name.StartsWith("player")) {
					int p = 6; while (p < r.name.GetCount() && IsDigit(r.name[p])) p++;
					player_id = StrInt(r.name.Mid(6, p - 6));
				}
				if (player_id >= 0) {
					while (state.players.GetCount() <= player_id) state.players.Add();
					state.players[player_id].is_active = active;
				}
			}
		}
	}
	for (int k = 0; k < card_rects.GetCount(); k++) {
		int i = card_indices[k]; const GameRule& r = rules[i];
		if (has_cards && k < card_mats.GetCount() && !card_mats[k].IsEmpty()) {
			const ByteMat& sub = card_mats[k];
			int card = card_clf.ClassifyUnified(sub);
			if (card >= 0 && card_clf.GetConfidence(card) > 0.2) {
				if (r.type == RULE_BOARD_CARD) state.community_cards.Add(card);
				else if (r.type == RULE_PLAYER_CARD || r.type == RULE_HAND_CARD) {
					int player_id = -1;
					if (r.name.StartsWith("player")) {
						int p = 6; while (p < r.name.GetCount() && IsDigit(r.name[p])) p++;
						player_id = StrInt(r.name.Mid(6, p - 6));
					}
					if (player_id >= 0) {
						while (state.players.GetCount() <= player_id) state.players.Add();
						state.players[player_id].hand.Add(card);
					}
				}
			}
		}
	}
	static Vector<int> last_board_cards;
	static Vector<int> last_hero_cards;
	static int board_stable_frames = 0;
	static int hero_stable_frames = 0;
	if (state.community_cards == last_board_cards) board_stable_frames++;
	else { last_board_cards <<= state.community_cards; board_stable_frames = 0; }
	Vector<int> hero_cards;
	if (state.players.GetCount() > 0) hero_cards <<= state.players[0].hand;
	if (hero_cards == last_hero_cards) hero_stable_frames++;
	else { last_hero_cards <<= hero_cards; hero_stable_frames = 0; }
	if (board_stable_frames >= 2 && hero_stable_frames >= 2) {
		state.round = scene_machine.Update(state);
		hh_builder.Update(state);
		
		const HandHistory& hh = hh_builder.GetCurrentHand();
		state.history.Clear();
		for (const auto& a : hh.actions) {
			int tok = GetActionToken(a.action);
			if (tok >= 0) state.history.Add((byte)tok);
		}

		if (eval_pimpl && strategy_pimpl && (state.players.GetCount() > 0 && state.players[0].hand.GetCount() == 2)) {
			const auto& hole = state.players[0].hand;
			const auto& board = state.community_cards;
			state.hero_equity = ComputeHeroEquity(hole, board, eval_pimpl);
			if (state.my_turn) {
				state.advice = GetStrategyAdvice(hole, board, state.pot, state.history, strategy_pimpl, state.advice_probs);
			}
		}
	} else {
		state.community_cards.Clear();
		if (state.players.GetCount() > 0) state.players[0].hand.Clear();
	}
	state.found = true;
	return state;
}

Rect RecognitionEngine::FindPokerTH(const Image& frame) { return frame.GetSize(); }

GameRound SceneMachine::Update(const GameState& state) {
	GameRound detected = ROUND_NONE;
	int cards = state.community_cards.GetCount();
	if (cards == 0) detected = ROUND_PREFLOP;
	else if (cards == 3) detected = ROUND_FLOP;
	else if (cards == 4) detected = ROUND_TURN;
	else if (cards == 5) detected = ROUND_RIVER;
	current_round = detected;
	return current_round;
}

String SceneMachine::GetRoundName() const {
	switch (current_round) {
		case ROUND_PREFLOP: return "Pre-flop";
		case ROUND_FLOP: return "Flop";
		case ROUND_TURN: return "Turn";
		case ROUND_RIVER: return "River";
		default: return "None";
	}
}

void RuleManager::Clear() { platforms.Clear(); active_platform.Clear(); }
void RuleManager::SetRules(Array<GameRule>&& src) { platforms.GetAdd(active_platform).rules = pick(src); }
Array<GameRule>& RuleManager::GetRules() { return platforms.GetAdd(active_platform).rules; }
const Array<GameRule>& RuleManager::GetRules() const {
	int q = platforms.Find(active_platform);
	if (q >= 0) return platforms[q].rules;
	static Array<GameRule> empty; return empty;
}

void RuleManager::Load(const String& p) {
	path = p;
	Clear();
	LoadFromJson(*this, LoadFile(p));
}

void RuleManager::Save() { SaveFile(path, StoreAsJson(*this)); }

int RuleManager::FindRule(const String& name) const {
	const auto& rules = GetRules();
	for (int i = 0; i < rules.GetCount(); i++) if (rules[i].name == name) return i;
	return -1;
}

Size RuleManager::GetBaseSize(const String& name) const {
	int q = platforms.Find(name);
	return q >= 0 ? platforms[q].base_size : Size(0, 0);
}

Rect RuleManager::GetAbsRect(int index, Size frame_size) const {
	const auto& rules = GetRules();
	if (index < 0 || index >= rules.GetCount()) return Rect(0, 0, 0, 0);
	const GameRule& r = rules[index];
	Size base = GetBaseSize(active_platform);
	if (base.cx <= 0 || base.cy <= 0) return r.rect;
	double sx = (double)frame_size.cx / base.cx;
	double sy = (double)frame_size.cy / base.cy;
	return Rect((int)(r.rect.left * sx), (int)(r.rect.top * sy), (int)(r.rect.right * sx), (int)(r.rect.bottom * sy));
}

void RuleManager::AddPlatform(const String& name, Size sz) {
	Platform& p = platforms.GetAdd(name);
	p.name = name; p.base_size = sz;
	active_platform = name;
}

void HandHistoryBuilder::Update(const GameState& state) {
	if (state.round != last_round) {
		last_round = state.round;
		current_hand.community_cards <<= state.community_cards;
	}
	while (last_player_actions.GetCount() < state.players.GetCount())
		last_player_actions.Add("");
	
	for (int i = 0; i < state.players.GetCount(); i++) {
		const auto& p = state.players[i];
		while (current_hand.players.GetCount() <= i) current_hand.players.Add();
		auto& hp = current_hand.players[i];
		hp.hand <<= p.hand;
		if (!p.name.IsEmpty()) hp.name = p.name;
		
		if (!p.last_action.IsEmpty() && p.last_action != last_player_actions[i]) {
			HandAction& a = current_hand.actions.Add();
			a.player_id = i;
			a.action = p.last_action;
			a.amount = p.bet;
			a.round = state.round;
			last_player_actions[i] = p.last_action;
		}
	}
}

void StateStability::Update(const GameState& state) {
	uint64 h = state.GetHash();
	if (h == last_hash) {
		stable_frames++;
		if (stable_frames == required_frames) {
			stable_state = state;
		}
	} else {
		last_hash = h;
		stable_frames = 0;
	}
}

int HysteresisFilter::Filter(const String& key, int new_value) {
	Entry& e = values.GetAdd(key);
	if (new_value == e.value) {
		e.count = 0;
	} else {
		e.count++;
		if (e.count >= threshold) {
			e.value = new_value;
			e.count = 0;
		}
	}
	return e.value;
}

}
