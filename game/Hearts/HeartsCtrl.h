#ifndef _game_Hearts_HeartsCtrl_h_
#define _game_Hearts_HeartsCtrl_h_

struct VisualCard : public Moveable<VisualCard> {
	Card card;
	Rect rect;
};

struct AnimCard : public Moveable<AnimCard> {
	Card card;
	Point src;
	Point dst;
	double progress; // 0.0 to 1.0
	int angle;
	bool back;
};

class HeartsCtrl : public Ctrl {
public:
	typedef HeartsCtrl CLASSNAME;

	enum Difficulty {
		DIFFICULTY_EASY,
		DIFFICULTY_MEDIUM,
		DIFFICULTY_HARD
	};

	GameState state;
	Difficulty difficulty;

	// UI Buttons
	Button btn_pass;
	Button btn_clear;

	// Animation state
	Vector<AnimCard> anim_cards;
	bool animating;

	// Game loop settings
	Vector<Card> selected_cards;
	bool autoplay_enabled;
	bool started;
	int pending_pass_player;
	bool pass_animating;
	bool collecting_trick;
	int collecting_winner;
	int collecting_points;

	HeartsCtrl();

	void StartGame();
	void ResetGame();
	void RefreshUI();
	void UpdateHUD();
	void ScheduleAiStep(int delay_ms);
	void AiStep();
	void StartTrickCollect();
	void FinishTrickCollect();
	void StartPassAnimation();
	void FinishPassAnimation();
	void NextRound();
	void FinishAutoplayIfNeeded();
	void ShowHighScores();
	void ShowAbout();

	void OnCardClick(const Card& card);
	void OnPassClick();
	void OnClearClick();

	// Layout and Paint
	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword flags) override;
	virtual void Layout() override;

private:
	Vector<VisualCard> human_card_rects; // Cached for hit testing

	void AnimateStep();
	void AddCardAnimation(const Card& card, Point src, Point dst, int angle, bool back = false);
	Point GetHandCenter(int player_idx) const;
	Point GetTrickCenter(int player_idx) const;
	Image LoadCardImage(const String& card_id);
};

#endif
