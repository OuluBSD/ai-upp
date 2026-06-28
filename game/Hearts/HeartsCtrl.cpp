#include "Hearts.h"

NAMESPACE_UPP

Image RotateCardImage(const Image& img, int rotation_deg) {
	int rot = rotation_deg % 360;
	if (rot < 0) rot += 360;
	if (rot == 90) return RotateClockwise(img);
	if (rot == 180) return Rotate180(img);
	if (rot == 270) return RotateAntiClockwise(img);
	return img;
}

HeartsCtrl::HeartsCtrl() {
	Add(btn_pass);
	Add(btn_clear);

	btn_pass.SetLabel("Pass Cards");
	btn_clear.SetLabel("Clear Selection");

	btn_pass.WhenAction = THISBACK(OnPassClick);
	btn_clear.WhenAction = THISBACK(OnClearClick);

	autoplay_enabled = false;
	started = false;
	pending_pass_player = -1;
	pass_animating = false;
	collecting_trick = false;
	collecting_winner = -1;
	collecting_points = 0;
	animating = false;
	difficulty = DIFFICULTY_HARD;

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
	
	if (state.phase == "PASSING") {
		state.SelectPass(1, HeartsAI::ChoosePassCards(state.players[1]));
		state.SelectPass(2, HeartsAI::ChoosePassCards(state.players[2]));
		state.SelectPass(3, HeartsAI::ChoosePassCards(state.players[3]));
	} else if (state.phase == "PLAYING" && state.turn != 0) {
		ScheduleAiStep(500);
	}
	
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
	if (state.trick_pending) return;

	if (state.phase == "PLAYING" && state.turn != 0) {
		int turn = state.turn;
		const Vector<Card>& hand = state.players[turn];
		Card chosen;

		if (difficulty == DIFFICULTY_EASY) {
			Vector<Card> valid;
			for (const Card& c : hand) {
				if (state.ValidatePlay(turn, c).success) {
					valid.Add(c);
				}
			}
			if (!valid.IsEmpty()) {
				chosen = valid[Random(valid.GetCount())];
			}
		} else if (difficulty == DIFFICULTY_MEDIUM) {
			Vector<Card> valid;
			for (const Card& c : hand) {
				if (state.ValidatePlay(turn, c).success) {
					valid.Add(c);
				}
			}
			if (!valid.IsEmpty()) {
				chosen = valid[0];
				int r = RankIndex(chosen.rank);
				for (int i = 1; i < valid.GetCount(); i++) {
					int r2 = RankIndex(valid[i].rank);
					if (r2 < r) {
						r = r2;
						chosen = valid[i];
					}
				}
			}
		} else {
			chosen = HeartsAI::ChooseCard(turn, hand, state.trick, state.leading_suit, state.hearts_broken);
		}

		if (!chosen.id.IsEmpty()) {
			state.PlayCard(turn, chosen);
			RefreshUI();
			
			if (state.trick_pending) {
				SetTimeCallback(1000, THISBACK(FinishTrickCollect), 2);
			} else if (state.turn != 0) {
				ScheduleAiStep(500);
			}
		}
	}
}

void HeartsCtrl::FinishTrickCollect() {
	state.ResolveTrick();
	RefreshUI();
	
	if (state.phase == "PLAYING" && state.turn != 0) {
		ScheduleAiStep(500);
	} else if (state.phase == "ROUND_END") {
		state.BeginNextRound();
		if (state.phase == "PASSING") {
			state.SelectPass(1, HeartsAI::ChoosePassCards(state.players[1]));
			state.SelectPass(2, HeartsAI::ChoosePassCards(state.players[2]));
			state.SelectPass(3, HeartsAI::ChoosePassCards(state.players[3]));
		} else if (state.phase == "PLAYING" && state.turn != 0) {
			ScheduleAiStep(500);
		}
		RefreshUI();
	}
}

void HeartsCtrl::StartTrickCollect() {}
void HeartsCtrl::StartPassAnimation() {}
void HeartsCtrl::FinishPassAnimation() {}
void HeartsCtrl::NextRound() {}
void HeartsCtrl::FinishAutoplayIfNeeded() {}
void HeartsCtrl::AnimateStep() {}
void HeartsCtrl::AddCardAnimation(const Card& card, Point src, Point dst, int angle, bool back) {}

Point HeartsCtrl::GetHandCenter(int player_idx) const {
	int cx = GetSize().cx;
	int cy = GetSize().cy;
	int centerX = cx / 2;
	int centerY = cy / 2;
	
	switch (player_idx) {
		case 0: return Point(centerX, cy - 80); // South
		case 1: return Point(cx - 80, centerY); // East
		case 2: return Point(centerX, 80);      // North
		case 3: return Point(80, centerY);       // West
	}
	return Point(0, 0);
}

Point HeartsCtrl::GetTrickCenter(int player_idx) const {
	int cx = GetSize().cx;
	int cy = GetSize().cy;
	int centerX = cx / 2;
	int centerY = cy / 2;
	int offset = min(cx, cy) * 0.12;
	
	switch (player_idx) {
		case 0: return Point(centerX, centerY + offset);
		case 1: return Point(centerX + offset, centerY);
		case 2: return Point(centerX, centerY - offset);
		case 3: return Point(centerX - offset, centerY);
	}
	return Point(0, 0);
}

void HeartsCtrl::ShowHighScores() {
	TopWindow win;
	win.Title("High Scores 🏆");
	win.SetRect(0, 0, 300, 200);
	
	Label lbl;
	lbl.SetText("1. Spearhead - 0 pts\n2. Director - 15 pts\n3. Curator - 34 pts\n4. Human - 48 pts");
	lbl.SetAlign(ALIGN_CENTER);
	win.Add(lbl.SizePos());
	
	win.Run();
}

void HeartsCtrl::ShowAbout() {
	PromptOK("[A1 Hearts Royal 💎]\n\nC++ Hearts Game implementation using U++ framework.\n\nCreated by Spearhead, 2026.");
}

void HeartsCtrl::OnCardClick(const Card& card) {
	if (state.phase == "PASSING") {
		int idx = FindIndex(selected_cards, card);
		if (idx != -1) {
			selected_cards.Remove(idx);
		} else {
			if (selected_cards.GetCount() < 3) {
				selected_cards.Add(card);
			}
		}
		RefreshUI();
	} else if (state.phase == "PLAYING" && state.turn == 0) {
		PlayResult res = state.PlayCard(0, card);
		if (res.success) {
			RefreshUI();
			if (state.trick_pending) {
				SetTimeCallback(1000, THISBACK(FinishTrickCollect), 2);
			} else if (state.turn != 0) {
				ScheduleAiStep(500);
			}
		} else {
			Exclamation(res.message);
		}
	}
}

void HeartsCtrl::OnPassClick() {
	if (state.phase == "PASSING" && selected_cards.GetCount() == 3) {
		state.SelectPass(0, selected_cards);
		selected_cards.Clear();
		RefreshUI();
		
		if (state.phase == "PLAYING" && state.turn != 0) {
			ScheduleAiStep(500);
		}
	}
}

void HeartsCtrl::OnClearClick() {
	selected_cards.Clear();
	RefreshUI();
}

void HeartsCtrl::Paint(Draw& w) {
	// Card metrics configuration
	static const int CARD_WIDTH = 71;
	static const int CARD_HEIGHT = 96;
	static const int HAND_SPACING = 25;

	w.DrawRect(GetSize(), Color(20, 100, 40)); // Felt green
	DrawFatFrame(w, GetSize(), Color(80, 50, 20), 8); // Wood border

	w.DrawText(20, 20, "HEARTS ROYAL 💎", Arial(20).Bold(), Color(255, 215, 0));

	if (!started) {
		int cx = GetSize().cx;
		int cy = GetSize().cy;
		w.DrawText(cx / 2 - 150, cy / 2 - 10, "Click Game -> New Game to start playing!", Arial(16).Bold().Italic(), White());
		return;
	}

	int cx = GetSize().cx;
	int cy = GetSize().cy;
	int centerX = cx / 2;
	int centerY = cy / 2;

	// Draw player label tags
	for (int i = 0; i < 4; i++) {
		Point handCenter = GetHandCenter(i);
		String name_tag;
		if (i == 0) name_tag = "You (P0)";
		else name_tag = Format("AI (P%d)", i);

		String info = Format("%s | %d pts", name_tag, state.scores[i]);
		if (state.phase == "PLAYING" && state.turn == i) {
			info += " ◄";
		}
		
		int ty = handCenter.y + (i == 2 ? 65 : -75);
		int tx = handCenter.x - 60;
		w.DrawText(tx, ty, info, Arial(14).Bold(), (state.turn == i) ? Color(255, 215, 0) : White());
	}

	human_card_rects.Clear();

	// Draw player hands
	for (int i = 0; i < 4; i++) {
		Point px = GetHandCenter(i);
		const Vector<Card>& hand = state.players[i];
		int num_cards = hand.GetCount();
		if (num_cards == 0) continue;

		int start_offset = - ((num_cards - 1) * HAND_SPACING) / 2;

		for (int j = 0; j < num_cards; j++) {
			Card card = hand[j];
			int cx_card = px.x;
			int cy_card = px.y;

			if (i == 0) { // South (Human)
				cx_card = px.x + start_offset + j * HAND_SPACING;
				cy_card = px.y;
				if (FindIndex(selected_cards, card) != -1) {
					cy_card -= 15;
				}
				
				Rect r(cx_card - CARD_WIDTH/2, cy_card - CARD_HEIGHT/2, cx_card + CARD_WIDTH/2, cy_card + CARD_HEIGHT/2);
				w.DrawImage(r.left, r.top, LoadCardImage(card.id));
				
				VisualCard vc;
				vc.card = card;
				vc.rect = r;
				human_card_rects.Add(vc);
			} else { // AIs (backs)
				if (i == 2) { // North
					cx_card = px.x + start_offset + j * 15;
					cy_card = px.y;
					w.DrawImage(cx_card - CARD_WIDTH/2, cy_card - CARD_HEIGHT/2, LoadCardImage("back9"));
				} else if (i == 1) { // East
					cx_card = px.x;
					cy_card = px.y + start_offset + j * 15;
					Image img = RotateCardImage(LoadCardImage("back9"), 90);
					w.DrawImage(cx_card - img.GetWidth()/2, cy_card - img.GetHeight()/2, img);
				} else { // West
					cx_card = px.x;
					cy_card = px.y + start_offset + j * 15;
					Image img = RotateCardImage(LoadCardImage("back9"), 270);
					w.DrawImage(cx_card - img.GetWidth()/2, cy_card - img.GetHeight()/2, img);
				}
			}
		}
	}

	// Draw trick cards
	for (int i = 0; i < state.trick.GetCount(); i++) {
		TrickItem item = state.trick[i];
		Point pt = GetTrickCenter(item.player_index);
		w.DrawImage(pt.x - CARD_WIDTH/2, pt.y - CARD_HEIGHT/2, LoadCardImage(item.card.id));
	}

	// Draw game info
	String phase_info = "Phase: " + state.phase;
	if (state.phase == "PASSING") {
		int pass_direction = state.round_number % 4;
		if (pass_direction == 1) phase_info += " (Pass Left)";
		else if (pass_direction == 2) phase_info += " (Pass Right)";
		else if (pass_direction == 3) phase_info += " (Pass Across)";
	}
	w.DrawText(20, cy - 30, phase_info, Arial(14), LtGray());

	if (state.game_over) {
		w.DrawRect(centerX - 200, centerY - 60, 400, 120, Color(30, 30, 30));
		w.DrawText(centerX - 100, centerY - 40, "GAME OVER!", Arial(28).Bold(), Color(255, 100, 100));

		int winner = 0;
		int min_score = state.scores[0];
		for (int i = 1; i < 4; i++) {
			if (state.scores[i] < min_score) {
				min_score = state.scores[i];
				winner = i;
			}
		}
		w.DrawText(centerX - 120, centerY + 10, Format("Player %d Wins with %d points!", winner, min_score), Arial(16), Color(255, 215, 0));
	}
}

void HeartsCtrl::LeftDown(Point p, dword flags) {
	if (!started || state.game_over) return;
	
	if (state.phase == "PASSING" || (state.phase == "PLAYING" && state.turn == 0)) {
		for (int i = human_card_rects.GetCount() - 1; i >= 0; i--) {
			if (human_card_rects[i].rect.Contains(p)) {
				OnCardClick(human_card_rects[i].card);
				break;
			}
		}
	}
}

void HeartsCtrl::Layout() {
	int centerX = GetSize().cx / 2;
	int centerY = GetSize().cy / 2;
	btn_pass.SetRect(centerX - 110, centerY + 120, 100, 30);
	btn_clear.SetRect(centerX + 10, centerY + 120, 100, 30);
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
	if (!FileExists(path)) {
		path = "C:/Users/sblo/Dev/PKR/data/gfx/cards/default/" + filename;
	}

	Image img = StreamRaster::LoadFileAny(path);
	cache.Add(card_id, img);
	return img;
}

END_UPP_NAMESPACE