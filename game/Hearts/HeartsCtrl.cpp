#include "Hearts.h"

NAMESPACE_UPP

static int SuitOrderValue(const String& suit) {
	if (suit == "clubs") return 0;
	if (suit == "diamonds") return 1;
	if (suit == "hearts") return 2;
	if (suit == "spades") return 3;
	return 99;
}

static int RankOrderValue(const String& rank) {
	static const char* RANKS[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
	for (int i = 0; i < 13; i++) {
		if (RANKS[i] == rank) return i;
	}
	return 99;
}

Image RotateCardImage(const Image& img, int rotation_deg) {
	int rot = rotation_deg % 360;
	if (rot < 0) rot += 360;
	if (rot == 90) return RotateClockwise(img);
	if (rot == 180) return Rotate180(img);
	if (rot == 270) return RotateAntiClockwise(img);
	return img;
}

HeartsCtrl::HeartsCtrl() {
	// Add Pass & Clear buttons
	Add(btn_pass);
	Add(btn_clear);

	btn_pass.SetLabel("Pass Cards");
	btn_clear.SetLabel("Clear");

	btn_pass.WhenAction = THISBACK(OnPassClick);
	btn_clear.WhenAction = THISBACK(OnClearClick);

	difficulty = DIFFICULTY_HARD;
	autoplay_enabled = false;
	started = false;
	pending_pass_player = -1;
	pass_animating = false;
	collecting_trick = false;
	collecting_winner = -1;
	collecting_points = 0;
	animating = false;

	BackPaint();
}

void HeartsCtrl::StartGame() {
	ResetGame();
	state.LogCallback = [=](const String& msg) {
		Cout() << msg << "\n";
		LOG(msg);
	};
	state.Deal();
	started = true;
	RefreshUI();
}

void HeartsCtrl::ResetGame() {
	anim_cards.Clear();
	animating = false;
	selected_cards.Clear();
	started = false;
	pending_pass_player = -1;
	pass_animating = false;
	collecting_trick = false;
	collecting_winner = -1;
	collecting_points = 0;
	KillTimeCallback(1);
	KillTimeCallback(2);
}

void HeartsCtrl::RefreshUI() {
	UpdateHUD();
	Refresh();
}

void HeartsCtrl::UpdateHUD() {
	bool show_pass = (state.phase == "PASSING" && pending_pass_player < 0);
	if (show_pass) {
		btn_pass.Show();
		btn_clear.Show();
		btn_pass.Enable(selected_cards.GetCount() == 3);
		btn_clear.Enable(selected_cards.GetCount() > 0);
	} else {
		btn_pass.Hide();
		btn_clear.Hide();
	}
}

void HeartsCtrl::ScheduleAiStep(int delay_ms) {
	KillTimeCallback(1);
	SetTimeCallback(delay_ms, THISBACK(AiStep), 1);
}

void HeartsCtrl::AiStep() {
	if (state.game_over) {
		FinishAutoplayIfNeeded();
		return;
	}
	if (pass_animating) return;

	if (state.trick_pending) {
		if (!collecting_trick) {
			StartTrickCollect();
		}
		return;
	}

	if (state.phase == "PASSING") {
		if (pending_pass_player < 0) {
			if (autoplay_enabled) {
				pending_pass_player = 0;
			} else {
				return;
			}
		}

		if (pending_pass_player >= 4) {
			pending_pass_player = -1;
			if (autoplay_enabled || state.turn != 0) {
				ScheduleAiStep(500);
			}
			return;
		}

		Vector<Card> hand;
		hand <<= state.players[pending_pass_player];
		Vector<Card> pass = HeartsAI::ChoosePassCards(hand);
		state.SelectPass(pending_pass_player, pass);

		pending_pass_player++;
		if (pending_pass_player >= 4) {
			pending_pass_player = -1;
		}

		if (state.phase == "PLAYING") {
			RefreshUI();
			StartPassAnimation();
			return;
		}

		RefreshUI();
		if (state.phase == "PASSING" || autoplay_enabled || state.turn != 0) {
			ScheduleAiStep(500);
		}
		return;
	}

	if (state.phase != "PLAYING") return;
	if (!autoplay_enabled && state.turn == 0) return;

	int p_idx = state.turn;
	Card card;
	if (difficulty == DIFFICULTY_EASY) {
		Vector<Card> valid_cards;
		for (const auto& c : state.players[p_idx]) {
			if (state.ValidatePlay(p_idx, c).success) {
				valid_cards.Add(c);
			}
		}
		if (valid_cards.GetCount() > 0) {
			card = valid_cards[Random(valid_cards.GetCount())];
		}
	} else if (difficulty == DIFFICULTY_MEDIUM) {
		Vector<Card> valid_cards;
		for (const auto& c : state.players[p_idx]) {
			if (state.ValidatePlay(p_idx, c).success) {
				valid_cards.Add(c);
			}
		}
		if (valid_cards.GetCount() > 0) {
			card = valid_cards[0];
			for (int i = 1; i < valid_cards.GetCount(); i++) {
				if (RankOrderValue(valid_cards[i].rank) < RankOrderValue(card.rank)) {
					card = valid_cards[i];
				}
			}
		}
	} else {
		card = HeartsAI::ChooseCard(p_idx, state.players[p_idx], state.trick, state.leading_suit, state.hearts_broken);
	}
	if (card.id.IsEmpty()) return;

	PlayResult r = state.PlayCard(p_idx, card);
	if (r.success) {
		Point src = GetHandCenter(p_idx);
		Point dst = GetTrickCenter(p_idx);
		int angle = (p_idx == 1) ? 90 : ((p_idx == 2) ? 0 : ((p_idx == 3) ? 270 : 0));
		AddCardAnimation(card, src, dst, angle, false); // Animation shows face card
		RefreshUI();

		if (state.trick_pending) {
			ScheduleAiStep(900);
		} else if (autoplay_enabled || state.turn != 0) {
			ScheduleAiStep(500);
		}
	} else {
		state.Log("AI move failed: " + r.message);
		ScheduleAiStep(500);
	}
}

void HeartsCtrl::StartTrickCollect() {
	if (!state.trick_pending) return;
	collecting_trick = true;
	collecting_winner = state.pending_trick_winner;
	collecting_points = state.pending_trick_points;

	Point dst = GetHandCenter(collecting_winner);
	for (const auto& item : state.trick) {
		Point src = GetTrickCenter(item.player_index);
		AddCardAnimation(item.card, src, dst, 0);
	}

	RefreshUI();
	SetTimeCallback(700, THISBACK(FinishTrickCollect), 1);
}

void HeartsCtrl::FinishTrickCollect() {
	if (!state.trick_pending) {
		collecting_trick = false;
		collecting_winner = -1;
		collecting_points = 0;
		return;
	}
	state.ResolveTrick();
	collecting_trick = false;
	collecting_winner = -1;
	collecting_points = 0;

	RefreshUI();

	if (state.phase == "ROUND_END") {
		SetTimeCallback(2500, THISBACK(NextRound), 1);
		return;
	}

	if (autoplay_enabled || state.turn != 0) {
		ScheduleAiStep(500);
	}
}

void HeartsCtrl::StartPassAnimation() {
	pass_animating = true;

	int pass_dir = state.round_number % 4;
	int offset = 0;
	if (pass_dir == 1) offset = 1;
	else if (pass_dir == 2) offset = -1;
	else if (pass_dir == 3) offset = 2;
	else return;

	for (int i = 0; i < 4; i++) {
		int target = (i + offset + 4) % 4;
		Point src = GetHandCenter(i);
		Point dst = GetHandCenter(target);

		for (int j = 0; j < state.passed_cards[i].GetCount(); j++) {
			Card card = state.passed_cards[i][j];
			AddCardAnimation(card, src, dst, 0, i != 0); // show back for opponents
		}
	}

	SetTimeCallback(700, THISBACK(FinishPassAnimation), 1);
}

void HeartsCtrl::FinishPassAnimation() {
	pass_animating = false;
	anim_cards.Clear();
	animating = false;
	RefreshUI();
	if (autoplay_enabled || state.turn != 0) {
		ScheduleAiStep(500);
	}
}

void HeartsCtrl::NextRound() {
	if (state.game_over) {
		FinishAutoplayIfNeeded();
		return;
	}
	if (state.phase != "ROUND_END") return;
	state.BeginNextRound();
	RefreshUI();
	if (autoplay_enabled) {
		ScheduleAiStep(500);
	}
}

void HeartsCtrl::FinishAutoplayIfNeeded() {
	state.Log("--- Final Scores ---");
	for (int i = 0; i < 4; i++) {
		state.Log("Player " + AsString(i) + ": " + AsString(state.scores[i]) + " points");
	}
}

void HeartsCtrl::OnCardClick(const Card& card) {
	if (pass_animating) return;

	if (state.phase == "PASSING") {
		int idx = -1;
		for (int i = 0; i < selected_cards.GetCount(); i++) {
			if (selected_cards[i].id == card.id) {
				idx = i;
				break;
			}
		}
		if (idx >= 0) {
			selected_cards.Remove(idx);
		} else if (selected_cards.GetCount() < 3) {
			selected_cards.Add(card);
		}
		RefreshUI();
	} else {
		PlayResult r = state.PlayCard(0, card);
		if (r.success) {
			AddCardAnimation(card, GetHandCenter(0), GetTrickCenter(0), 0);
			RefreshUI();

			if (state.trick_pending) {
				ScheduleAiStep(900);
			} else if (state.turn != 0) {
				ScheduleAiStep(500);
			}
		} else {
			state.Log("Invalid move: " + r.message);
		}
	}
}

void HeartsCtrl::OnPassClick() {
	if (selected_cards.GetCount() != 3) return;

	Vector<Card> pass;
	pass <<= selected_cards;
	selected_cards.Clear();
	state.SelectPass(0, pass);

	RefreshUI();
	pending_pass_player = 1;
	ScheduleAiStep(500);
}

void HeartsCtrl::OnClearClick() {
	selected_cards.Clear();
	RefreshUI();
}

static void SortCardsForHand(Vector<Card>& cards) {
	for (int i = 0; i < cards.GetCount(); i++) {
		int best = i;
		for (int j = i + 1; j < cards.GetCount(); j++) {
			int suit_i = SuitOrderValue(cards[best].suit);
			int suit_j = SuitOrderValue(cards[j].suit);
			int rank_i = RankOrderValue(cards[best].rank);
			int rank_j = RankOrderValue(cards[j].rank);

			bool before = false;
			if (suit_j < suit_i) {
				before = true;
			} else if (suit_j == suit_i) {
				if (cards[j].GetPoints() < cards[best].GetPoints()) {
					before = true;
				} else if (cards[j].GetPoints() == cards[best].GetPoints() && rank_j < rank_i) {
					before = true;
				}
			}
			if (before) {
				best = j;
			}
		}
		if (best != i) {
			Swap(cards[i], cards[best]);
		}
	}
}

void HeartsCtrl::Paint(Draw& w) {
	// 1. Draw green felt table background
	w.DrawRect(GetSize(), Color(34, 112, 63));
	
	// Circular outline detail
	w.DrawEllipse(Rect(GetSize()).Deflated(20), Null, 3, Color(24, 82, 46));

	if (!started) {
		w.DrawText(GetSize().cx/2 - 100, GetSize().cy/2 - 10, "Hearts Standalone Game", StdFont(20).Bold(), White());
		return;
	}

	Point center = GetSize() / 2;
	int card_w = 71;
	int card_h = 96;

	// Animation blocker / lookup
	auto IsAnimating = [this](const String& card_id) {
		for (const auto& ac : anim_cards) {
			if (ac.card.id == card_id) return true;
		}
		return false;
	};

	// 2. Draw opponent hands (West, North, East)
	// West (player 1) - stack vertically, rotated 90 degrees
	{
		int count = state.players[1].GetCount();
		Point hc = GetHandCenter(1);
		Image back_img = RotateCardImage(LoadCardImage("back9"), 90);
		int bh = back_img.GetHeight();
		int bw = back_img.GetWidth();
		int start_y = hc.y - (count * 15 + bh) / 2;
		for (int i = 0; i < count; i++) {
			Card card = state.players[1][i];
			if (IsAnimating(card.id)) continue;
			w.DrawImage(hc.x - bw/2, start_y + i * 15, back_img);
		}
	}

	// North (player 2) - stack horizontally, facing down
	{
		int count = state.players[2].GetCount();
		Point hc = GetHandCenter(2);
		Image back_img = LoadCardImage("back9");
		int bh = back_img.GetHeight();
		int bw = back_img.GetWidth();
		int start_x = hc.x - (count * 15 + bw) / 2;
		for (int i = 0; i < count; i++) {
			Card card = state.players[2][i];
			if (IsAnimating(card.id)) continue;
			w.DrawImage(start_x + i * 15, hc.y - bh/2, back_img);
		}
	}

	// East (player 3) - stack vertically, rotated -90 degrees (270)
	{
		int count = state.players[3].GetCount();
		Point hc = GetHandCenter(3);
		Image back_img = RotateCardImage(LoadCardImage("back9"), 270);
		int bh = back_img.GetHeight();
		int bw = back_img.GetWidth();
		int start_y = hc.y - (count * 15 + bh) / 2;
		for (int i = 0; i < count; i++) {
			Card card = state.players[3][i];
			if (IsAnimating(card.id)) continue;
			w.DrawImage(hc.x - bw/2, start_y + i * 15, back_img);
		}
	}

	// 3. Draw human player hand (player 0)
	{
		human_card_rects.Clear();
		SortCardsForHand(state.players[0]);
		int count = state.players[0].GetCount();
		Point hc = GetHandCenter(0);
		int step = count > 1 ? (((GetSize().cx - 200) / count < 30) ? ((GetSize().cx - 200) / count) : 30) : 30;
		int start_x = hc.x - (count * step + card_w) / 2;
		for (int i = 0; i < count; i++) {
			Card card = state.players[0][i];
			if (IsAnimating(card.id)) continue;

			int cy = hc.y - card_h/2;
			// Pop up selected card
			bool selected = false;
			for (const auto& sc : selected_cards) {
				if (sc.id == card.id) {
					selected = true;
					break;
				}
			}
			if (selected) {
				cy -= 20;
			}

			Image img = LoadCardImage(card.id);
			int cx = start_x + i * step;
			w.DrawImage(cx, cy, img);

			VisualCard vc;
			vc.card = card;
			vc.rect = Rect(cx, cy, cx + card_w, cy + card_h);
			human_card_rects.Add(vc);
		}
	}

	// 4. Draw played cards in trick area
	for (const auto& item : state.trick) {
		if (IsAnimating(item.card.id)) continue;
		Image img = LoadCardImage(item.card.id);
		Point tp = GetTrickCenter(item.player_index);
		w.DrawImage(tp.x - img.GetWidth()/2, tp.y - img.GetHeight()/2, img);
	}

	// 5. Draw active animation cards
	for (const auto& ac : anim_cards) {
		Image img = ac.back ? LoadCardImage("back9") : LoadCardImage(ac.card.id);
		img = RotateCardImage(img, ac.angle);
		int cur_x = ac.src.x + int((ac.dst.x - ac.src.x) * ac.progress);
		int cur_y = ac.src.y + int((ac.dst.y - ac.src.y) * ac.progress);
		w.DrawImage(cur_x - img.GetWidth()/2, cur_y - img.GetHeight()/2, img);
	}

	// 6. Draw HUD labels for players
	static const char* PLAYER_NAMES[] = { "You", "West", "North", "East" };
	for (int i = 0; i < 4; i++) {
		Point hc = GetHandCenter(i);
		int text_y = 0;
		int text_x = 0;
		if (i == 0) { text_x = hc.x - 70; text_y = hc.y - card_h/2 - 25; }
		else if (i == 1) { text_x = hc.x + card_h/2 + 10; text_y = hc.y - 10; }
		else if (i == 2) { text_x = hc.x - 70; text_y = hc.y + card_h/2 + 10; }
		else if (i == 3) { text_x = hc.x - card_h/2 - 130; text_y = hc.y - 10; }

		String name = PLAYER_NAMES[i];
		if (state.phase == "PLAYING" && state.turn == i) {
			name = "[" + name + "]";
		}

		String info = name + "  T:" + AsString(state.scores[i]) + " R:+" + AsString(state.round_scores[i]);
		
		// Highlight box for active turn
		if (state.phase == "PLAYING" && state.turn == i) {
			w.DrawRect(text_x - 5, text_y - 2, 130, 20, Color(220, 160, 40));
			w.DrawText(text_x, text_y, info, StdFont(13).Bold(), Black());
		} else {
			w.DrawText(text_x, text_y, info, StdFont(13).Bold(), White());
		}
	}

	// 7. Scoreboard overlay (ROUND_END / GAME_OVER)
	if (state.phase == "ROUND_END") {
		int box_w = 260;
		int box_h = 160;
		int box_x = center.x - box_w/2;
		int box_y = center.y - box_h/2;
		w.DrawRect(box_x, box_y, box_w, box_h, Color(30, 30, 30));
		DrawFrame(w, box_x, box_y, box_w, box_h, Black());
		DrawFrame(w, box_x + 1, box_y + 1, box_w - 2, box_h - 2, Black());
		
		w.DrawText(box_x + 20, box_y + 15, "Round " + AsString(state.round_number) + " complete", StdFont(15).Bold(), Yellow());
		
		for (int i = 0; i < 4; i++) {
			String line = String(PLAYER_NAMES[i]) + "   +" + AsString(state.last_round_scores[i]) + " (Total: " + AsString(state.scores[i]) + ")";
			w.DrawText(box_x + 20, box_y + 45 + i * 20, line, StdFont(13), White());
		}
		if (state.last_round_moon_shooter >= 0) {
			w.DrawText(box_x + 20, box_y + 130, "MOON SHOT by " + String(PLAYER_NAMES[state.last_round_moon_shooter]) + "!", StdFont(13).Bold(), Color(255, 120, 120));
		}
	} else if (state.phase == "GAME_OVER") {
		int box_w = 260;
		int box_h = 160;
		int box_x = center.x - box_w/2;
		int box_y = center.y - box_h/2;
		w.DrawRect(box_x, box_y, box_w, box_h, Color(30, 30, 30));
		DrawFrame(w, box_x, box_y, box_w, box_h, Black());
		DrawFrame(w, box_x + 1, box_y + 1, box_w - 2, box_h - 2, Black());

		w.DrawText(box_x + 20, box_y + 15, "GAME OVER", StdFont(16).Bold(), Color(255, 60, 60));

		int winner = 0;
		for (int i = 1; i < 4; i++) {
			if (state.scores[i] < state.scores[winner]) {
				winner = i;
			}
		}
		w.DrawText(box_x + 20, box_y + 45, "Winner: " + String(PLAYER_NAMES[winner]), StdFont(14).Bold(), Color(120, 255, 120));
		for (int i = 0; i < 4; i++) {
			w.DrawText(box_x + 20, box_y + 75 + i * 18, String(PLAYER_NAMES[i]) + ": " + AsString(state.scores[i]) + " pts", StdFont(13), White());
		}
	}

	// 8. Status Bar at the bottom
	String status_msg = "";
	if (state.phase == "PASSING") {
		int pass_dir = state.round_number % 4;
		String dir_text = (pass_dir == 1) ? "left" : ((pass_dir == 2) ? "right" : "across");
		if (pass_dir == 0) {
			status_msg = "Round " + AsString(state.round_number) + ": no passing. Starting play.";
		} else {
			status_msg = "Round " + AsString(state.round_number) + ": select 3 cards to pass " + dir_text + " (" + AsString(selected_cards.GetCount()) + "/3)";
		}
	} else if (state.phase == "PLAYING") {
		String prefix = (state.turn == 0) ? "Your turn" : "Waiting for " + String(PLAYER_NAMES[state.turn]);
		if (state.trick_pending) prefix = "Resolving trick";
		if (collecting_trick && collecting_winner >= 0) prefix = "Collecting trick for " + String(PLAYER_NAMES[collecting_winner]);
		String broken = state.hearts_broken ? "hearts broken" : "hearts closed";
		status_msg = "Round " + AsString(state.round_number) + ": " + prefix + ". Trick " + AsString(state.trick.GetCount()) + ", " + broken;
	} else if (state.phase == "ROUND_END") {
		status_msg = "Round complete. Next round starts soon...";
	} else if (state.phase == "GAME_OVER") {
		status_msg = "Game over!";
	}
	
	w.DrawText(15, GetSize().cy - 25, status_msg, StdFont(13).Bold(), White());
}

void HeartsCtrl::LeftDown(Point p, dword flags) {
	if (animating || pass_animating || collecting_trick) return;

	for (int i = human_card_rects.GetCount() - 1; i >= 0; i--) {
		if (human_card_rects[i].rect.Contains(p)) {
			OnCardClick(human_card_rects[i].card);
			return;
		}
	}
}

void HeartsCtrl::Layout() {
	Point center = GetSize() / 2;
	btn_pass.SetRect(GetSize().cx - 240, GetSize().cy - 35, 110, 25);
	btn_clear.SetRect(GetSize().cx - 120, GetSize().cy - 35, 100, 25);
}

Point HeartsCtrl::GetHandCenter(int player_idx) const {
	Point center = GetSize() / 2;
	// Accommodate margins
	int rx = center.x - 70;
	int ry = center.y - 70;

	switch (player_idx) {
		case 0: return Point(center.x, center.y + ry);     // South (bottom)
		case 1: return Point(center.x - rx, center.y);     // West (left)
		case 2: return Point(center.x, center.y - ry);     // North (top)
		case 3: return Point(center.x + rx, center.y);     // East (right)
	}
	return center;
}

Point HeartsCtrl::GetTrickCenter(int player_idx) const {
	Point center = GetSize() / 2;
	int ox = 50;
	int oy = 40;

	switch (player_idx) {
		case 0: return Point(center.x, center.y + oy);     // South
		case 1: return Point(center.x - ox, center.y);     // West
		case 2: return Point(center.x, center.y - oy);     // North
		case 3: return Point(center.x + ox, center.y);     // East
	}
	return center;
}

void HeartsCtrl::AddCardAnimation(const Card& card, Point src, Point dst, int angle, bool back) {
	AnimCard ac;
	ac.card = card;
	ac.src = src;
	ac.dst = dst;
	ac.progress = 0.0;
	ac.angle = angle;
	ac.back = back;
	anim_cards.Add(ac);

	if (!animating) {
		animating = true;
		SetTimeCallback(20, THISBACK(AnimateStep), 2);
	}
}

void HeartsCtrl::AnimateStep() {
	bool done = true;
	for (int i = 0; i < anim_cards.GetCount(); i++) {
		anim_cards[i].progress += 0.08;
		if (anim_cards[i].progress < 1.0) {
			done = false;
		} else {
			anim_cards[i].progress = 1.0;
		}
	}

	Refresh();

	if (!done) {
		SetTimeCallback(20, THISBACK(AnimateStep), 2);
	} else {
		anim_cards.Clear();
		animating = false;
	}
}

Image HeartsCtrl::LoadCardImage(const String& card_id) {
	static VectorMap<String, Image> cache;
	int idx = cache.Find(card_id);
	if (idx >= 0) return cache[idx];

	String filename = card_id + ".png";
	String path = AppendFileName(GetFileDirectory(GetExeFilePath()), "../share/imgs/cards/default/" + filename);
	if (!FileExists(path)) {
		path = AppendFileName(GetCurrentDirectory(), "share/imgs/cards/default/" + filename);
	}
	if (!FileExists(path)) {
		path = "C:/Users/sblo/Dev/ai-upp/share/imgs/cards/default/" + filename;
	}

	Image img = StreamRaster::LoadFileAny(path);
	cache.Add(card_id, img);
	return img;
}

void HeartsCtrl::ShowHighScores() {
	TopWindow d;
	d.Title("High Scores");
	d.SetRect(0, 0, 300, 220);

	Label lbl;
	lbl.SetText("Hearts Royal \360\237\222\216 High Scores\n\n1. Spearhead   - 12 pts\n2. Captain     - 25 pts\n3. Ringleader  - 42 pts\n4. You         - 58 pts\n5. Director    - 71 pts");
	lbl.SetAlign(ALIGN_CENTER);
	lbl.SetFont(StdFont(14));

	d.Add(lbl.SizePos());

	Button btn_ok;
	btn_ok.SetLabel("Close");
	btn_ok.WhenAction = d.Breaker();
	d.Add(btn_ok.BottomPos(10, 25).RightPos(10, 80));

	d.Run();
}

void HeartsCtrl::ShowAbout() {
	TopWindow d;
	d.Title("About Hearts Royal");
	d.SetRect(0, 0, 350, 160);

	Label lbl;
	lbl.SetText("Hearts Royal \360\237\222\216 v1.0\n\nBuilt using the Ultimate++ C++ Framework.\n\nEnjoy playing classic Hearts card game with beautiful animations and difficulty settings.");
	lbl.SetAlign(ALIGN_CENTER);
	lbl.SetFont(StdFont(13));

	d.Add(lbl.SizePos());

	Button btn_ok;
	btn_ok.SetLabel("OK");
	btn_ok.WhenAction = d.Breaker();
	d.Add(btn_ok.BottomPos(10, 25).RightPos(10, 80));

	d.Run();
}

END_UPP_NAMESPACE
