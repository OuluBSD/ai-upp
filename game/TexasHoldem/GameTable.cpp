#include "GameTable.h"
#include "ClientThread.h"
#include "NetPacket.h"
#include <Poker/CardsValue.h>
#include <EditorCommon/Tools.h>
#include <EditorCommon/EditorCommon.h>
#include <EditorCommon/GameTheme.h>
#include <EditorCommon/GameThemeRender.h>
#include <Poker/TexasRenderer.h>
#include <Poker/TableLayoutProfile.h>
#include <GameRules/Game.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BoardInterface.h>
#include <GameRules/BeroInterface.h>
#include <GameRules/PlayerInterface.h>
#include "SettingsWindow.h"
#include <EditorCommon/ConfigFile.h>
#include <plugin/png/png.h>
#include <cmath>

NAMESPACE_UPP

extern std::shared_ptr<ClientThread> g_clientThread;

static Image SwapRBForPng(const Image& src)
{
	if (src.IsEmpty())
		return src;
	Image tmp = src;
	ImageBuffer ib(tmp);
	for (int y = 0; y < ib.GetHeight(); y++) {
		RGBA* row = ib[y];
		for (int x = 0; x < ib.GetWidth(); x++) {
			byte t = row[x].r;
			row[x].r = row[x].b;
			row[x].b = t;
		}
	}
	return Image(ib);
}

static bool SaveSnapshotPng(const String& path, const Image& src)
{
	Image out = src;
#ifdef flagX11
	// X11 screenshot/export path stores channels in opposite order for PNG output.
	// Keep explicit swap here so saved artifacts match on-screen colors.
	out = SwapRBForPng(out);
#endif
	RealizeDirectory(GetFileFolder(path));
	return PNGEncoder().SaveFile(path, out);
}

PokerChatCtrl::PokerChatCtrl()
{
	Add(chatHistory);
	Add(chatInput);
	Add(btnSend);
	chatHistory.SetReadOnly();
	btnSend.SetLabel(t_("Send"));
}

void PokerChatCtrl::Layout()
{
	Size sz = GetSize();
	int btnW = 60;
	int editH = 20;
	chatHistory.SetRect(0, 0, sz.cx, sz.cy - editH - 2);
	chatInput.SetRect(0, sz.cy - editH, sz.cx - btnW - 2, editH);
	btnSend.SetRect(sz.cx - btnW, sz.cy - editH, btnW, editH);
}

Image GameTable::ApplyInterlacedTransparency(const Image& img)
{
	Image m = img;
	ImageBuffer ib(m);
	for(int y = 0; y < ib.GetHeight(); y++) {
		if (y % 2 != 0) {
			for(int x = 0; x < ib.GetWidth(); x++)
				ib[y][x] = RGBAZero();
		}
	}
	return ib;
}

Image GameTable::GetCardImage(int card)
{
	static String cachedTheme;
	static VectorMap<int, Image> cache;
	if (cachedTheme != currentCardTheme) {
		cache.Clear();
		cachedTheme = currentCardTheme;
	}
	if (cache.Find(card) >= 0) return cache.Get(card);

	String filename;
	if (card < 0) {
		filename = "back9.png";
	} else {
		static const char* suits[] = { "clubs", "diamonds", "hearts", "spades" };
		static const char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace" };
		if (card < 52) {
			filename = String(suits[card / 13]) + "_" + ranks[card % 13] + ".png";
		}
	}

	String path = AppendFileName(GetFileDirectory(GetExeFilePath()), "../share/imgs/cards/default/" + filename);
	if (!FileExists(path)) {
		path = AppendFileName(GetCurrentDirectory(), "share/imgs/cards/default/" + filename);
	}
	if (!FileExists(path)) {
		path = "C:/Users/sblo/Dev/ai-upp/share/imgs/cards/default/" + filename;
	}

	Image img = StreamRaster::LoadFileAny(path);
	if (!img.IsEmpty()) {
		img = Rescale(img, Size(48, 76));
	} else {
		ImageDraw iw(48, 76);
		iw.Alpha().DrawRect(0, 0, 48, 76, RGBAZero());
		iw.DrawRect(0, 0, 48, 76, (card < 0 && card != -2) ? Color(0, 0, 150) : White());
		DrawFrame(iw, 0, 0, 48, 76, Black());
		if (card >= 0) {
			Font f = SansSerif(14).Bold();
			Size tsz = GetTextSize(AsString(card), f);
			iw.DrawText((48 - tsz.cx) / 2, (76 - tsz.cy) / 2, AsString(card), f, Black());
		} else if (card == -2) {
			iw.DrawRect(4, 4, 40, 68, Color(0, 0, 150));
		}
		img = iw;
	}
	cache.Add(card, img);
	return img;
}

Image GameTable::GetTransparentCard(int card, int alpha)
{
	return ApplyInterlacedTransparency(GetCardImage(card));
}

PlayerCtrl::PlayerCtrl()
{
	Transparent();
	Add(bg);
	Add(label_Avatar);
	Add(pixmapLabel_carda);
	Add(pixmapLabel_cardb);
	Add(pixmapLabel_cards);
	Add(label_PlayerName);
	Add(textLabel_Cash);
	Add(textLabel_Set);
	Add(textLabel_Button);
	Add(actionPic);
	Add(label_Timeout);

	pixmapLabel_cards.Transparent();
	pixmapLabel_cards.Hide();
	label_PlayerName.Transparent();
	textLabel_Cash.Transparent();
	textLabel_Set.Transparent();
	label_Timeout.Transparent();

	label_PlayerName.SetInk(White());
	textLabel_Cash.SetInk(Yellow());
	textLabel_Set.SetInk(Yellow());
	label_Timeout.Color(SColorFace());
}

void PlayerCtrl::Layout()
{
	Size sz = GetSize();
	double fx = (double)sz.cx / 190.0;
	double fy = (double)sz.cy / 142.0;

	bg.SetRect(0, 0, sz.cx, sz.cy);
	label_Avatar.SetRect((int)(10 * fx), (int)(10 * fy), max(1, (int)(60 * fx)), max(1, (int)(60 * fy)));
	label_PlayerName.SetRect((int)(10 * fx), (int)(75 * fy), max(1, (int)(170 * fx)), max(1, (int)(15 * fy)));
	textLabel_Cash.SetRect((int)(10 * fx), (int)(90 * fy), max(1, (int)(170 * fx)), max(1, (int)(15 * fy)));
	
	pixmapLabel_carda.SetRect((int)(80 * fx), (int)(10 * fy), max(1, (int)(48 * fx)), max(1, (int)(60 * fy)));
	pixmapLabel_cardb.SetRect((int)(110 * fx), (int)(10 * fy), max(1, (int)(48 * fx)), max(1, (int)(60 * fy)));
	pixmapLabel_cards.SetRect((int)(80 * fx), (int)(10 * fy), max(1, (int)(78 * fx)), max(1, (int)(60 * fy)));
	
	textLabel_Button.SetRect((int)(140 * fx), (int)(75 * fy), max(1, (int)(32 * fx)), max(1, (int)(32 * fy)));
	textLabel_Set.SetRect((int)(10 * fx), (int)(105 * fy), max(1, (int)(170 * fx)), max(1, (int)(15 * fy)));
	
	actionPic.SetRect((int)(10 * fx), (int)(120 * fy), max(1, (int)(170 * fx)), max(1, (int)(20 * fy)));
	label_Timeout.SetRect((int)(10 * fx), (int)(70 * fy), max(1, (int)(60 * fx)), max(1, (int)(5 * fy)));
}

GameTable::GameTable()
{
	// BOARD FIRST - IMPORTANT FOR Z-ORDER
	Add(board);
	
	Add(lblPotTitle); Add(lblPotTotal); Add(lblPotBets);
	Add(lblTurnTitle); Add(lblGameInfo); Add(lblHandInfo);
	Add(tabLeft); Add(tabRight);
	
	Add(humanButtons);
	humanButtons.Transparent();
	humanButtons.Add(btnFold); humanButtons.Add(btnCheckCall); humanButtons.Add(btnBetRaise);
	humanButtons.Add(btnAllIn); humanButtons.Add(btnPot33); humanButtons.Add(btnPot50);
	humanButtons.Add(btnPot100); humanButtons.Add(editBet); humanButtons.Add(sliderBet);
	
	Add(lblSpeed); Add(sliderSpeed); Add(btnPause);
	
	for(int i = 0; i < 10; i++) Add(players[i]);

	Sizeable().Zoomable();
	Title(t_("PKR Game Table"));
	AddFrame(menu);
	menu.Set(callback(this, &GameTable::MainMenu));

	LoadTheme("default");

	tabLeft.Add(chatCtrl.SizePos(), t_("Chat"));
	handsImgPane.Background(Color(0, 60, 0));
	handsImgPane.Add(handsImg.SizePos());
	tabLeft.Add(handsImgPane.SizePos(), t_("Hands"));
	
	tabRight.Add(engineLogView.SizePos(), t_("Log"));
	tabRight.Add(probCtrl.SizePos(), t_("Probabilities"));
	tabRight.Add(awayLog.SizePos(), t_("Away"));
	
	sliderBet.WhenAction = [this] {
		editBet.SetData(sliderBet.GetData());
		UpdateActionLabels();
	};
	editBet.WhenAction = [this] {
		sliderBet.SetData(editBet.GetData());
		UpdateActionLabels();
	};

	btnFold.WhenAction = callback(this, &GameTable::OnFold);
	btnCheckCall.WhenAction = callback(this, &GameTable::OnCheckCall);
	btnBetRaise.WhenAction = callback(this, &GameTable::OnBetRaise);
	btnAllIn.WhenAction = callback(this, &GameTable::OnAllIn);
	
	btnFold.SetLabel(t_("Fold")); btnFold.shortcut = "F1";
	btnCheckCall.SetLabel(t_("Check")); btnCheckCall.shortcut = "F2";
	btnBetRaise.SetLabel(t_("Raise")); btnBetRaise.shortcut = "F3";
	btnAllIn.SetLabel(t_("All-In")); btnAllIn.shortcut = "F4";
	
	btnPot33.WhenAction = callback(this, &GameTable::OnPot33);
	btnPot50.WhenAction = callback(this, &GameTable::OnPot50);
	btnPot100.WhenAction = callback(this, &GameTable::OnPot100);
	
	btnPot33.SetLabel("33%");
	btnPot50.SetLabel("50%");
	btnPot100.SetLabel("100%");

	humanButtons.Hide();
	
	m_script.action_proxy = [this](int act) {
		switch(act) {
			case 0: OnFold(); break;
			case 1: OnCheckCall(); break;
			case 2: OnBetRaise(); break;
			case 3: OnAllIn(); break;
		}
	};
	SetProjectContext("default", "texas-holdem");

	SetTimeCallback(-500, [this] { Timer(); });

	chatCtrl.btnSend << [this] {
		String msg = chatCtrl.chatInput.GetData();
		if (!msg.IsEmpty() && m_game && m_game->getPlayerByNumber(0)) {
			chatCtrl.chatHistory.Append("[" + m_game->getPlayerByNumber(0)->getMyName() + "] " + msg + "\n");
			chatCtrl.chatInput.SetData("");
		}
	};

	SColorPaper_Write(Color(0, 60, 0));
	SColorText_Write(White());
	SColorFace_Write(Color(0, 60, 0));

	lblPotTitle.SetInk(Color(255, 255, 120)).SetAlign(ALIGN_CENTER).SetFont(StdFont().Bold());
	lblPotTitle = t_("Pot");
	lblTurnTitle.SetInk(Color(255, 255, 120)).SetAlign(ALIGN_CENTER).SetFont(StdFont().Bold());
	lblPotTotal.SetInk(Yellow()).SetAlign(ALIGN_CENTER);
	lblPotBets.SetInk(Yellow()).SetAlign(ALIGN_CENTER);
	lblGameInfo.SetInk(Yellow()).SetAlign(ALIGN_CENTER);
	lblHandInfo.SetInk(Yellow()).SetAlign(ALIGN_CENTER);

	lblSpeed = t_("Speed: 4");
	sliderSpeed.Range(10).SetData(4);
	sliderSpeed.WhenAction = callback(this, &GameTable::OnSpeedChange);
	btnPause.WhenAction = callback(this, &GameTable::OnPause);
	btnPause.SetLabel(t_("Pause"));
	
	board.WhenPaint = [this](Draw& w) { PaintBoard(w); };
	
	for (int i = 0; i < 5; i++) boardCards[i] = -1;
}

GameTable::~GameTable() {}

void GameTable::Layout()
{
	Size sz = GetSize();
	if (sz.cx <= 0 || sz.cy <= 0) return;
	
	board.SetRect(0, 0, sz.cx, sz.cy);
	
	double fx = (double)sz.cx / 1024.0;
	double fy = (double)sz.cy / 648.0;

	for (int i = 0; i < 10; i++)
	{
		players[i].SetRect(TexasTableLayout::PlayerRect(sz, i));
		players[i].Layout();
	}

	Rect pr = TexasTableLayout::PotRect(sz);
	lblPotTitle.SetRect(pr.left, max(0, pr.top - (int)(40 * fy)), pr.GetWidth(), max(12, (int)(20 * fy)));
	lblPotTotal.SetRect(pr.left, max(0, pr.top - (int)(18 * fy)), pr.GetWidth(), max(12, (int)(16 * fy)));
	lblPotBets.SetRect(pr.left, pr.top, pr.GetWidth(), max(12, (int)(16 * fy)));

	Point rt = TexasTableLayout::RoundTextPos(sz);
	lblTurnTitle.SetRect(rt.x, rt.y, max(100, (int)(200 * fx)), max(16, (int)(20 * fy)));
	lblGameInfo.SetRect(rt.x, rt.y + max(16, (int)(22 * fy)), max(100, (int)(200 * fx)), max(12, (int)(16 * fy)));
	lblHandInfo.SetRect(rt.x, rt.y + max(30, (int)(40 * fy)), max(100, (int)(200 * fx)), max(12, (int)(16 * fy)));

	tabLeft.SetRect(2 * fx, 456 * fy, 260 * fx, 190 * fy);
	tabRight.SetRect(762 * fx, 456 * fy, 260 * fx, 190 * fy);
	
	humanButtons.SetRect(428 * fx, 436 * fy, 166 * fx, 206 * fy);
	
	// HumanButtons Relative Layout
	double hbx = fx;
	double hby = fy;
	
	btnAllIn.SetRect(0, 0, 92 * hbx, 30 * hby);
	editBet.SetRect(92 * hbx, 0, 74 * hbx, 30 * hby);
	sliderBet.SetRect(0, 30 * hby, 168 * hbx, 20 * hby);
	btnPot33.SetRect(0, 50 * hby, 56 * hbx, 24 * hby);
	btnPot50.SetRect(58 * hbx, 50 * hby, 56 * hbx, 24 * hby);
	btnPot100.SetRect(114 * hbx, 50 * hby, 56 * hbx, 24 * hby);
	btnBetRaise.SetRect(0, 74 * hby, 168 * hbx, 44 * hby);
	btnCheckCall.SetRect(0, 118 * hby, 168 * hbx, 44 * hby);
	btnFold.SetRect(0, 162 * hby, 168 * hbx, 44 * hby);

	lblSpeed.SetRect(762 * fx, 430 * fy, 80 * fx, 20 * fy);
	sliderSpeed.SetRect(842 * fx, 430 * fy, 100 * fx, 20 * fy);
	btnPause.SetRect(942 * fx, 430 * fy, 80 * fx, 20 * fy);
	
	Refresh();
}

void GameTable::SetGame(std::shared_ptr<class Game> game)
{
	m_game = game;
	if (m_game && m_game->getConfig()) {
		String theme = m_game->getConfig()->readConfigString("TableTheme");
		if (theme.IsEmpty()) theme = "default";
		LoadTheme(theme);
	}
}

void GameTable::MainMenu(Bar& bar)
{
	bar.Add(t_("Actions"), callback(this, &GameTable::ActionsMenu));
	bar.Add(t_("View"), callback(this, &GameTable::ViewMenu));
	bar.Add(t_("Show"), callback(this, &GameTable::ShowMenu));
	bar.Add(t_("Settings"), callback(this, &GameTable::SettingsMenu));
}

void GameTable::ViewMenu(Bar& bar)
{
	bar.Add(t_("Normal View"), callback(this, &GameTable::OnViewNormal)).Key(K_F7);
	bar.Add(t_("Image View"), callback(this, &GameTable::OnViewImage)).Key(K_F8);
	bar.Separator();
	bar.Add(t_("Save Screenshot"), callback(this, &GameTable::OnSaveScreenshot)).Key(K_F9);
}

void GameTable::OnSaveScreenshot()
{
	String path = AppendFileName(GetCurrentDirectory(), "tmp/pokerth_screenshot.png");
	if (DumpSnapshot(path, true))
		AppendLog("Screenshot saved to " + path);
}

bool GameTable::DumpSnapshot(const String& path, bool image_mode)
{
	ImageDraw id(GetSize());
	if (image_mode)
		RenderToImage(id);
	else
		RenderNormalSnapshot(id);
	Image img = id;
	return SaveSnapshotPng(path, img);
}

void GameTable::OnViewNormal()
{
	if (!m_imageMode) return;
	m_imageMode = false;
	
	// Show all standard UI controls
	board.Show();
	lblPotTitle.Show(); lblPotTotal.Show(); lblPotBets.Show();
	lblTurnTitle.Show(); lblGameInfo.Show(); lblHandInfo.Show();
	tabLeft.Show(); tabRight.Show();
	// humanButtons visibility depends on game state, but let's just refresh layout
	lblSpeed.Show(); sliderSpeed.Show(); btnPause.Show();
	for(int i = 0; i < 10; i++) players[i].Show();
	
	bool hero_turn = false;
	if (m_game && m_game->getCurrentHand()) {
		auto bero = m_game->getCurrentHand()->getCurrentBeRo();
		auto hero = m_game->getPlayerByNumber(0);
		if (bero && hero) {
			int tid = (int)bero->getCurrentPlayersTurnId();
			hero_turn = tid >= 0 && (unsigned)tid == hero->getMyUniqueID();
		}
	}
	if (hero_turn) humanButtons.Show();
	else humanButtons.Hide();

	UpdateActionLabels();
	Refresh();
}

void GameTable::OnViewImage()
{
	if (m_imageMode) return;
	m_imageMode = true;
	
	// Hide all standard UI controls
	board.Hide();
	lblPotTitle.Hide(); lblPotTotal.Hide(); lblPotBets.Hide();
	lblTurnTitle.Hide(); lblGameInfo.Hide(); lblHandInfo.Hide();
	tabLeft.Hide(); tabRight.Hide();
	humanButtons.Hide();
	lblSpeed.Hide(); sliderSpeed.Hide(); btnPause.Hide();
	for(int i = 0; i < 10; i++) players[i].Hide();
	
	Refresh();
}

void GameTable::ShowMenu(Bar& bar)
{
	bar.Add(t_("Full-screen"), callback(this, &GameTable::OnToggleFullScreen)).Key(K_CTRL_F);
	bar.Separator();
	bar.Add(t_("Show/hide chat window"), callback(this, &GameTable::OnToggleChat)).Key(K_CTRL_T);
	bar.Add(t_("Show/hide hands window"), callback(this, &GameTable::OnToggleHands)).Key(K_CTRL_H);
	bar.Add(t_("Show/hide log window"), callback(this, &GameTable::OnToggleLog)).Key(K_CTRL_L);
	bar.Add(t_("Show/hide away window"), callback(this, &GameTable::OnToggleAway)).Key(K_CTRL_A);
	bar.Add(t_("Show/hide probability window"), callback(this, &GameTable::OnToggleProbs));
	bar.Separator();
	bar.Add(t_("Exit"), callback(this, &GameTable::Close));
}

void GameTable::SettingsMenu(Bar& bar)
{
	bar.Add(t_("PKR settings"), callback(this, &GameTable::OnSettings));
}

void GameTable::ActionsMenu(Bar& bar)
{
	bar.Add(t_("Fold"), callback(this, &GameTable::OnFold)).Key(K_F1);
	bar.Add(t_("Check/Call"), callback(this, &GameTable::OnCheckCall)).Key(K_F2);
	bar.Add(t_("Bet/Raise"), callback(this, &GameTable::OnBetRaise)).Key(K_F3);
	bar.Add(t_("All-In"), callback(this, &GameTable::OnAllIn)).Key(K_F4);
}

void GameTable::ToggleTab(TabCtrl& tab, Ctrl& parent, int index)
{
	if (tab.Get() == index && tab.IsVisible()) {
		tab.Hide();
	} else {
		tab.Set(index);
		tab.Show();
	}
}

void GameTable::OnToggleFullScreen()
{
	FullScreen(!IsFullScreen());
}

void GameTable::OnToggleChat() { ToggleTab(tabLeft, board, 0); }
void GameTable::OnToggleHands() { ToggleTab(tabLeft, board, 1); }
void GameTable::OnToggleLog() { ToggleTab(tabRight, board, 0); }
void GameTable::OnToggleAway() { ToggleTab(tabRight, board, 2); }
void GameTable::OnToggleProbs() { ToggleTab(tabRight, board, 1); }

void GameTable::OnSpeedChange()
{
	lblSpeed.SetLabel(Format(t_("Speed: %d"), (int)sliderSpeed.GetData()));
}

int GameTable::GetActionDelayMs() const
{
	int speed = (int)sliderSpeed.GetData();
	return max(0, (10 - speed) * 200);
}

bool GameTable::IsActionFlowPaused() const
{
	return m_isPaused;
}

void GameTable::SetScriptAutomationEnabled(bool enabled)
{
	m_scriptAutomationEnabled = enabled;
}

void GameTable::SetProjectContext(const String& project_name, const String& platform_name)
{
	m_scriptProject = TrimBoth(project_name);
	m_scriptPlatform = TrimBoth(platform_name);
	if (m_scriptProject.IsEmpty())
		m_scriptProject = "default";
	if (m_scriptPlatform.IsEmpty())
		m_scriptPlatform = "texas-holdem";

	String code = LoadProjectCommonCode(m_scriptProject, String());
	String platform_code = LoadPlatformCode(m_scriptProject, m_scriptPlatform, String());
	if (!code.IsEmpty() && !platform_code.IsEmpty())
		code << "\n";
	code << platform_code;

	bool loaded = false;
	if (!code.IsEmpty())
		loaded = m_script.Load(code);
	if (!loaded) {
		String fallback = AppendFileName(AppendFileName(GetCurrentDirectory(), "gamescreen/default/platforms"), "texas-holdem.py");
		m_script.LoadFile(fallback);
		m_scriptPlatform = "texas-holdem";
	}
	ApplyProjectThemeMetadata(m_scriptProject, m_scriptPlatform);
}

void GameTable::ApplyProjectThemeMetadata(const String& project_name, const String& platform_name)
{
	String theme_path = GetProjectThemePath(project_name, platform_name);
	m_loadedTheme.objects.Clear();
	m_loadedTheme.settings.Clear();
	m_themeProfile = "default";
	TexasTableLayout::SetProfile(ResolveThemeLayoutProfile(m_loadedTheme, platform_name));
	if (!FileExists(theme_path))
		return;

	try {
		LoadFromJson(m_loadedTheme, LoadFile(theme_path));
	}
	catch (...) {
		m_loadedTheme.objects.Clear();
		m_loadedTheme.settings.Clear();
		return;
	}

	m_themeProfile = ResolveThemeProfile(m_loadedTheme);
	TexasTableLayout::SetProfile(ResolveThemeLayoutProfile(m_loadedTheme, platform_name));

	for (const ThemeObject& o : m_loadedTheme.objects) {
		if (!ThemeObjectVisibleForProfile(o, m_themeProfile))
			continue;
		String t = ToLower(TrimBoth(o.type));
		if (t == "background") {
			String p = ResolveThemeAssetPath(project_name, ThemeGetPropString(o.props, "asset"));
			Image bg = StreamRaster::LoadFileAny(p);
			if (!bg.IsEmpty())
				tableImg = bg;
		}
		else if (t == "card-theme") {
			String theme = ThemeGetPropString(o.props, "theme");
			if (!theme.IsEmpty())
				currentCardTheme = theme;
		}
	}
}

void GameTable::BuildThemeGameState(GameState& st) const
{
	st = GameState();
	if (!m_game)
		return;
	std::shared_ptr<HandInterface> hand = m_game->getCurrentHand();
	if (!hand)
		return;
	std::shared_ptr<BoardInterface> board = hand->getBoard();
	if (board) {
		st.pot = board->getPot();
		const int* cards = board->getMyCards();
		for (int i = 0; i < 5; i++) {
			if (cards[i] >= 0)
				st.community_cards.Add(cards[i]);
		}
	}
	for (int i = 0; i < m_game->getStartQuantityPlayers(); i++) {
		std::shared_ptr<PlayerInterface> p = m_game->getPlayerByNumber(i);
		if (!p)
			continue;
		GameState::Player& gp = st.players.Add();
		gp.name = p->getMyName();
		gp.stack = p->getMyCash();
		gp.bet = p->getMySet();
		gp.is_active = p->getMyActiveStatus();
		gp.is_dealer = p->getMyButton() == GBUTTON_DEALER;
		gp.is_folded = p->getMyAction() == PLAYER_ACTION_FOLD;
		gp.is_allin = p->getMyAction() == PLAYER_ACTION_ALLIN;
		st.stacks.Add(gp.stack);
	}
	switch (hand->getCurrentRound()) {
		case GAME_STATE_PREFLOP: st.round = ROUND_PREFLOP; break;
		case GAME_STATE_FLOP: st.round = ROUND_FLOP; break;
		case GAME_STATE_TURN: st.round = ROUND_TURN; break;
		case GAME_STATE_RIVER: st.round = ROUND_RIVER; break;
		case GAME_STATE_POST_RIVER: st.round = ROUND_SHOWDOWN; break;
		default: st.round = ROUND_NONE; break;
	}
	st.my_turn = false;
	std::shared_ptr<PlayerInterface> hero = m_game->getPlayerByNumber(0);
	if (hero)
		st.my_turn = hero->getMyTurn();
	st.active_platform = m_scriptPlatform;
}

void GameTable::OnPause()
{
	m_isPaused = !m_isPaused;
	btnPause.SetLabel(m_isPaused ? t_("Play") : t_("Pause"));
}

void GameTable::OnSettings()
{
	if (!m_game || !m_game->getConfig()) return;
	SettingsWindow dlg;
	dlg.Init(*m_game->getConfig());
	if (dlg.Run() == IDOK) {
		String theme = m_game->getConfig()->readConfigString("TableTheme");
		if (theme.IsEmpty()) theme = "default";
		LoadTheme(theme);
		Refresh();
	}
}

void GameTable::UpdateActionLabels()
{
	if (!m_game)
		return;
	auto hand = m_game->getCurrentHand();
	auto p = m_game->getPlayerByNumber(0);
	if (!hand || !p)
		return;
	auto bero = hand->getCurrentBeRo();
	if (!bero)
		return;

	int highestSet = bero->getHighestSet();
	int mySet = p->getMySet();
	int callValue = highestSet - mySet;
	if (callValue > p->getMyCash()) callValue = p->getMyCash();
	if (callValue < 0) callValue = 0;

	btnCheckCall.SetLabel(callValue > 0 ? String(t_("Call\n")) + Format("$%d", callValue) : String(t_("Check\n")));
	
	int currentBet = (int)editBet.GetData();
	int raiseAmount = currentBet - mySet;
	btnBetRaise.SetLabel(callValue > 0 ? String(t_("Raise\n")) + Format("$%d", raiseAmount) : String(t_("Bet\n")) + Format("$%d", raiseAmount));
}

void GameTable::OnFold()
{
	if (m_game) {
		auto hand = m_game->getCurrentHand();
		auto p = m_game->getPlayerByNumber(0);
		if (!p) {
			HideActionButtons();
			return;
		}
		if (hand)
			hand->setPreviousPlayerID(p->getMyID());
		p->setMyAction(PLAYER_ACTION_FOLD, true);
		logPlayerActionMsg(p->getMyName(), PLAYER_ACTION_FOLD, 0);
		nextPlayerAnimation();
	}
	HideActionButtons();
}

void GameTable::HideActionButtons()
{
	humanButtons.Hide();
}

void GameTable::OnCheckCall()
{
	if (m_game) {
		auto hand = m_game->getCurrentHand();
		auto p = m_game->getPlayerByNumber(0);
		auto bero = hand ? hand->getCurrentBeRo() : nullptr;
		if (!p || !bero) {
			HideActionButtons();
			return;
		}
		int highestSet = bero->getHighestSet();
		int mySetBefore = p->getMySet();
		int callValue = highestSet - mySetBefore;
		if (callValue > p->getMyCash()) callValue = p->getMyCash();
		if (callValue < 0) callValue = 0;
		p->setMySet(callValue);
		PlayerAction act;
		if (callValue > 0 && p->getMyCash() == 0) act = PLAYER_ACTION_ALLIN;
		else act = highestSet > mySetBefore ? PLAYER_ACTION_CALL : PLAYER_ACTION_CHECK;
		hand->setPreviousPlayerID(p->getMyID());
		p->setMyAction(act, true);
		logPlayerActionMsg(p->getMyName(), (int)act, callValue);
		nextPlayerAnimation();
	}
	HideActionButtons();
}

void GameTable::OnBetRaise()
{
	if (m_game) {
		auto hand = m_game->getCurrentHand();
		auto p = m_game->getPlayerByNumber(0);
		auto bero = hand ? hand->getCurrentBeRo() : nullptr;
		if (!p || !bero) {
			HideActionButtons();
			return;
		}
		int val = (int)editBet.GetData();
		int mySetBefore = p->getMySet();
		int raiseVal = val - mySetBefore;
		p->setMySet(raiseVal);
		if (p->getMySet() > bero->getHighestSet())
			bero->setHighestSet(p->getMySet());
		hand->setPreviousPlayerID(p->getMyID());
		PlayerAction act = (p->getMyCash() == 0) ? PLAYER_ACTION_ALLIN : PLAYER_ACTION_RAISE;
		p->setMyAction(act, true);
		logPlayerActionMsg(p->getMyName(), act, p->getMySet());
		nextPlayerAnimation();
	}
	HideActionButtons();
}

void GameTable::OnAllIn()
{
	if (m_game) {
		auto hand = m_game->getCurrentHand();
		auto p = m_game->getPlayerByNumber(0);
		auto bero = hand ? hand->getCurrentBeRo() : nullptr;
		if (!p || !bero) {
			HideActionButtons();
			return;
		}
		p->setMySet(p->getMyCash());
		if (p->getMySet() > bero->getHighestSet())
			bero->setHighestSet(p->getMySet());
		hand->setPreviousPlayerID(p->getMyID());
		p->setMyAction(PLAYER_ACTION_ALLIN, true);
		logPlayerActionMsg(p->getMyName(), PLAYER_ACTION_ALLIN, p->getMySet());
		nextPlayerAnimation();
	}
	HideActionButtons();
}

void GameTable::OnPot33()  {
	if (!m_game) return;
	auto hand = m_game->getCurrentHand();
	auto bero = hand ? hand->getCurrentBeRo() : nullptr;
	auto p = m_game->getPlayerByNumber(0);
	if (!bero || !p) return;
	int call = bero->getHighestSet() - p->getMySet();
	int min_raise = max(1, bero->getMinimumRaise());
	int min_total = p->getMySet() + call + min_raise;
	int max_opp_cash = 0;
	for (int i = 0; i < 10; i++) {
		auto q = m_game->getPlayerByNumber(i);
		if (!q || !q->getMyActiveStatus())
			continue;
		if (q->getMyID() != p->getMyID() && q->getMyAction() != PLAYER_ACTION_FOLD)
			max_opp_cash = max(max_opp_cash, q->getMyCash());
	}
	int effective_stack = min(p->getMyCash(), max_opp_cash);
	int target_total = p->getMySet() + max(call, effective_stack * 33 / 100);
	if (target_total > p->getMySet() && target_total < min_total)
		target_total = min_total;
	target_total = min(target_total, p->getMySet() + max(0, effective_stack));
	editBet.SetData(target_total);
	UpdateActionLabels();
}
void GameTable::OnPot50()  {
	if (!m_game) return;
	auto hand = m_game->getCurrentHand();
	auto bero = hand ? hand->getCurrentBeRo() : nullptr;
	auto p = m_game->getPlayerByNumber(0);
	if (!bero || !p) return;
	int call = bero->getHighestSet() - p->getMySet();
	int min_raise = max(1, bero->getMinimumRaise());
	int min_total = p->getMySet() + call + min_raise;
	int max_opp_cash = 0;
	for (int i = 0; i < 10; i++) {
		auto q = m_game->getPlayerByNumber(i);
		if (!q || !q->getMyActiveStatus())
			continue;
		if (q->getMyID() != p->getMyID() && q->getMyAction() != PLAYER_ACTION_FOLD)
			max_opp_cash = max(max_opp_cash, q->getMyCash());
	}
	int effective_stack = min(p->getMyCash(), max_opp_cash);
	int target_total = p->getMySet() + max(call, effective_stack * 50 / 100);
	if (target_total > p->getMySet() && target_total < min_total)
		target_total = min_total;
	target_total = min(target_total, p->getMySet() + max(0, effective_stack));
	editBet.SetData(target_total);
	UpdateActionLabels();
}
void GameTable::OnPot100() {
	if (!m_game) return;
	auto hand = m_game->getCurrentHand();
	auto bero = hand ? hand->getCurrentBeRo() : nullptr;
	auto p = m_game->getPlayerByNumber(0);
	if (!bero || !p) return;
	int call = bero->getHighestSet() - p->getMySet();
	int min_raise = max(1, bero->getMinimumRaise());
	int min_total = p->getMySet() + call + min_raise;
	int max_opp_cash = 0;
	for (int i = 0; i < 10; i++) {
		auto q = m_game->getPlayerByNumber(i);
		if (!q || !q->getMyActiveStatus())
			continue;
		if (q->getMyID() != p->getMyID() && q->getMyAction() != PLAYER_ACTION_FOLD)
			max_opp_cash = max(max_opp_cash, q->getMyCash());
	}
	int effective_stack = min(p->getMyCash(), max_opp_cash);
	int target_total = p->getMySet() + max(call, effective_stack);
	if (effective_stack > 0) target_total = p->getMySet() + effective_stack;
	if (target_total > p->getMySet() && target_total < min_total)
		target_total = min_total;
	target_total = min(target_total, p->getMySet() + max(0, effective_stack));
	editBet.SetData(target_total);
	UpdateActionLabels();
}

void GameTable::LoadTheme(const String& themeName)
{
	String dataDir = Tools::GetDataDir();
	String dir = AppendFileName(dataDir, "gfx/gui/table/" + themeName);
	Image img = StreamRaster::LoadFileAny(AppendFileName(dir, "table.png"));
	if (!img.IsEmpty())
		tableImg = img;
	puckDealer = StreamRaster::LoadFileAny(AppendFileName(dir, "dealerPuck.png"));
	puckSB = StreamRaster::LoadFileAny(AppendFileName(dir, "smallblindPuck.png"));
	puckBB = StreamRaster::LoadFileAny(AppendFileName(dir, "bigblindPuck.png"));
	avatarDefault = StreamRaster::LoadFileAny(AppendFileName(dir, "genereticAvatar.png"));
	cardHolderFlop = StreamRaster::LoadFileAny(AppendFileName(dir, "cardholder_flop.png"));
	cardHolderTurn = StreamRaster::LoadFileAny(AppendFileName(dir, "cardholder_turn.png"));
	cardHolderRiver = StreamRaster::LoadFileAny(AppendFileName(dir, "cardholder_river.png"));
	handsImg.SetImage(StreamRaster::LoadFileAny(AppendFileName(dir, "handranking.png")));
	
	String actionDir = AppendFileName(dataDir, "gfx/gui/misc/actionpics");
	actionPics[PLAYER_ACTION_CHECK] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_check.png"));
	actionPics[PLAYER_ACTION_CALL] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_call.png"));
	actionPics[PLAYER_ACTION_BET] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_bet.png"));
	actionPics[PLAYER_ACTION_RAISE] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_raise.png"));
	actionPics[PLAYER_ACTION_FOLD] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_fold.png"));
	actionPics[PLAYER_ACTION_ALLIN] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_allin.png"));
	actionPics[9] = StreamRaster::LoadFileAny(AppendFileName(actionDir, "action_winner.png"));
	
	if (m_game && m_game->getConfig()) {
		currentCardTheme = m_game->getConfig()->readConfigString("CardTheme");
	}
	if (currentCardTheme.IsEmpty()) currentCardTheme = "default_800x480";

	auto LoadBtn = [&](ActionButton& b, String color) {
		b.img = StreamRaster::LoadFileAny(AppendFileName(dir, "playeraction_" + color + ".png"));
		b.img_h = StreamRaster::LoadFileAny(AppendFileName(dir, "playeraction_" + color + "_hover.png"));
		b.img_p = StreamRaster::LoadFileAny(AppendFileName(dir, "playeraction_" + color + "_checked.png"));
	};
	
	LoadBtn(btnCheckCall, "blue");
	LoadBtn(btnBetRaise, "green");
	LoadBtn(btnFold, "red");
	LoadBtn(btnAllIn, "orange");
	LoadBtn(btnPot33, "green");
	LoadBtn(btnPot50, "green");
	LoadBtn(btnPot100, "green");
}

void GameTable::PaintBoard(Draw& w)
{
	Size sz = board.GetSize();
	auto DrawScaled = [&](int x, int y, int cx, int cy, const Image& img) {
		if (img.IsEmpty()) return;
		Size isz = img.GetSize();
		if (isz.cx == cx && isz.cy == cy)
			w.DrawImage(x, y, img);
		else
			w.DrawImage(x, y, Rescale(img, cx, cy));
	};
	if (!tableImg.IsEmpty())
		DrawScaled(0, 0, sz.cx, sz.cy, tableImg);
	else {
		w.DrawRect(sz, Color(0, 100, 0));
		w.DrawEllipse(sz.cx / 10, sz.cy / 10, 8 * sz.cx / 10, 8 * sz.cy / 10, Color(0, 80, 0), 2, Gray());
	}
	
	double fx = (double)sz.cx / 1024.0;
	double fy = (double)sz.cy / 648.0;
	
	for (int i = 0; i < 5; i++) {
		Rect br = TexasTableLayout::BoardCardRect(sz, i);
		int x = br.left;
		int y = br.top;
		int w1 = br.GetWidth();
		int h1 = br.GetHeight();
		if (boardCards[i] >= 0) {
			Image img = GetCardImage(boardCards[i]);
			DrawScaled(x, y, w1, h1, img);
		} else {
			Image holder;
			if (i < 3) holder = cardHolderFlop;
			else if (i == 3) holder = cardHolderTurn;
			else holder = cardHolderRiver;
			if (!holder.IsEmpty())
				DrawScaled(x, y, w1, h1, holder);
			else
				w.DrawRect(x, y, w1, h1, Color(0, 80, 0));
		}
	}
}

void GameTable::RenderToImage(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, Black()); // Clear background
	
	// 1. Draw Board (background + community cards)
	PaintBoard(w);
	
	// 2. Draw Pot Labels
	auto DrawLabel = [&](Label& lbl, Color ink = White()) {
		Rect r = lbl.GetRect();
		Image img = lbl.GetImage();
		if (!img.IsEmpty())
			w.DrawImage(r.left, r.top, img);
		String txt = lbl.GetText();
		if (!txt.IsEmpty())
			w.DrawText(r.left, r.top, txt, lbl.GetFont(), ink);
	};
	DrawLabel(lblPotTitle);
	DrawLabel(lblPotTotal, Yellow());
	DrawLabel(lblPotBets, Yellow());
	DrawLabel(lblTurnTitle);
	DrawLabel(lblGameInfo);
	DrawLabel(lblHandInfo);
	
	// 3. Draw Players
	for (int i = 0; i < 10; i++) {
		PlayerCtrl& p = players[i];
		
		Rect pr = p.GetRect();
		// Draw Player background manually or via child
		// Replicating PlayerBgCtrl::Paint
		bool winner = p.bg.isWinner;
		Color bc = winner ? Color(0, 255, 0) : Color(0, 120, 0);
		w.DrawRect(pr, Black());
		for(int y = 0; y < pr.GetHeight(); y += 2)
			w.DrawRect(pr.left, pr.top + y, pr.GetWidth(), 1, bc);
		DrawFrame(w, pr, winner ? Color(0, 255, 0) : Color(0, 180, 0));
		
		// Draw Player Children
		auto DrawChild = [&](Ctrl& c, Color ink = White()) {
			Rect cr = c.GetRect();
			Rect abs_cr = cr + pr.TopLeft();
			if (dynamic_cast<Label*>(&c)) {
				Label& l = (Label&)c;
				Image limg = l.GetImage();
				if (!limg.IsEmpty())
					w.DrawImage(abs_cr.left, abs_cr.top, limg);
				String txt = l.GetText();
				if (!txt.IsEmpty())
					w.DrawText(abs_cr.left, abs_cr.top, txt, l.GetFont(), ink);
			} else if (dynamic_cast<ScaledImageCtrl*>(&c)) {
				ScaledImageCtrl& si = (ScaledImageCtrl&)c;
				if (!si.img.IsEmpty()) {
					Size isz = si.img.GetSize();
					double r = fmin((double)cr.GetWidth() / isz.cx, (double)cr.GetHeight() / isz.cy);
					int w1 = (int)(isz.cx * r);
					int h1 = (int)(isz.cy * r);
					Image out = (w1 == isz.cx && h1 == isz.cy) ? si.img : Rescale(si.img, w1, h1);
					w.DrawImage(abs_cr.left + (cr.GetWidth() - w1) / 2, abs_cr.top + (cr.GetHeight() - h1) / 2, out);
				}
			}
		};
		
		DrawChild(p.label_PlayerName);
		DrawChild(p.textLabel_Cash, Yellow());
		DrawChild(p.textLabel_Set, Yellow());
		DrawChild(p.pixmapLabel_carda);
		DrawChild(p.pixmapLabel_cardb);
		DrawChild(p.pixmapLabel_cards);
		DrawChild(p.textLabel_Button, Yellow());
		DrawChild(p.actionPic);
	}
	
	// 4. Draw Game Log (12 lines)
	Font logFont = StdFont(12).Bold();
	int logY = sz.cy - 12 * (logFont.GetLineHeight() + 2) - 10;
	for (int i = 0; i < m_gameLog.GetCount(); i++) {
		Size tsz = GetTextSize(m_gameLog[i], logFont);
		w.DrawText(sz.cx - tsz.cx - 12, logY, m_gameLog[i], logFont, White());
		logY += logFont.GetLineHeight() + 2;
	}
}

void GameTable::RenderNormalSnapshot(Draw& w)
{
	RenderToImage(w);

	Font panel_font = StdFont(12).Bold();
	auto draw_panel = [&](const Rect& r, const String& title) {
		w.DrawRect(r, Color(35, 35, 35));
		DrawFrame(w, r, Color(90, 90, 90));
		if (!title.IsEmpty())
			w.DrawText(r.left + 6, r.top + 5, title, panel_font, White());
	};
	auto draw_button = [&](ActionButton& b) {
		Rect r = b.GetRect();
		Rect ar = r + humanButtons.GetRect().TopLeft();
		w.DrawRect(ar, Color(50, 90, 140));
		DrawFrame(w, ar, Color(140, 170, 220));
		Vector<String> lines = Split(AsString(b.GetLabel()), '\n');
		int y = ar.top + 4;
		for (const String& s : lines) {
			if (!s.IsEmpty())
				w.DrawText(ar.left + 6, y, s, panel_font, White());
			y += panel_font.GetLineHeight();
		}
	};

	draw_panel(tabLeft.GetRect(), "Tabs");
	draw_panel(tabRight.GetRect(), "Panels");
	draw_panel(humanButtons.GetRect(), "Actions");

	draw_button(btnAllIn);
	draw_button(btnPot33);
	draw_button(btnPot50);
	draw_button(btnPot100);
	draw_button(btnBetRaise);
	draw_button(btnCheckCall);
	draw_button(btnFold);

	Rect er = editBet.GetRect() + humanButtons.GetRect().TopLeft();
	w.DrawRect(er, White());
	DrawFrame(w, er, Color(80, 80, 80));
	w.DrawText(er.left + 4, er.top + 4, AsString((int)editBet.GetData()), panel_font, Black());

	Rect sb = sliderBet.GetRect() + humanButtons.GetRect().TopLeft();
	w.DrawRect(sb, Color(70, 70, 70));
	DrawFrame(w, sb, Color(120, 120, 120));
	Rect ss = sliderSpeed.GetRect();
	w.DrawRect(ss, Color(70, 70, 70));
	DrawFrame(w, ss, Color(120, 120, 120));
	w.DrawText(lblSpeed.GetRect().left, lblSpeed.GetRect().top, lblSpeed.GetText(), panel_font, White());

	Rect pr = btnPause.GetRect();
	w.DrawRect(pr, Color(90, 90, 90));
	DrawFrame(w, pr, Color(160, 160, 160));
	w.DrawText(pr.left + 6, pr.top + 4, btnPause.GetLabel(), panel_font, White());
}

void GameTable::Paint(Draw& w)
{
	if (m_imageMode) {
		RenderToImage(w);
	} else {
		TopWindow::Paint(w);
	}
}

void GameTable::Timer()
{
	if (!m_scriptAutomationEnabled) return;
	if (!m_game || !m_script.IsLoaded()) return;
	
	// 1. Render to Image
	ImageDraw id(GetSize());
	RenderToImage(id);
	Image img = id;
	
	// 2. Run Script
	PyValue instance = m_script.RunCommonMain(img);
	if (instance.IsNone()) return;
	
	Vector<Rect> rects;
	Vector<double> scores;
	m_script.RunPlatformMain(m_scriptPlatform, instance, img, PyValue::None(), rects, scores);
}

void GameTable::initGui(int speed)
{
	for (int i = 0; i < 10; i++) refreshGroupbox(i, 0);
	refreshPot();
	refreshSet();
}

void GameTable::refreshGameLabels(TexasRound state) const {}
void GameTable::nextRoundCleanGui()
{
	for (int i = 0; i < 5; i++) boardCards[i] = -1;
	board.Refresh();
	for (int i = 0; i < 10; i++) {
		players[i].textLabel_Set.SetLabel("");
		players[i].actionPic.SetImage(Null);
	}
}

void GameTable::AppendLog(const String& s)
{
	engineLogView.SetQTF(engineLogView.GetQTF() + DeQtf(s) + "&");
	engineLogView.ScrollEnd();
	
	m_gameLog.Add(s);
	while (m_gameLog.GetCount() > 12)
		m_gameLog.Remove(0);
	if (m_imageMode)
		Refresh();
}

void GameTable::logNewGameHandMsg(int gameID, int HandID)
{
	AppendLog(Format("## Game: %d | Hand: %d ##", gameID, HandID));
}

void GameTable::flushLogAtGame(int gameID) {}
void GameTable::logNewBlindsSetsMsg(int sbSet, int bbSet, String sbName, String bbName)
{
	AppendLog(Format("## Blinds: %d / %d ##", sbSet, bbSet));
	AppendLog(Format("%s posts small blind ($%d)", sbName, sbSet));
	AppendLog(Format("%s posts big blind ($%d)", bbName, bbSet));
}

void GameTable::flushLogAtHand() {}
void GameTable::dealHoleCards()
{
	for (int i = 0; i < 10; i++) refreshGroupbox(i, 0);
}

void GameTable::refreshPot()
{
	if (m_game && m_game->getCurrentHand() && m_game->getCurrentHand()->getBoard()) {
		int total = 0;
		int round = 0;
		for (int i = 0; i < 10; i++) {
			auto p = m_game->getPlayerByNumber(i);
			if (!p || !p->getMyActiveStatus())
				continue;
			total += max(0, p->getMyRoundStartCash() - p->getMyCash());
			round += max(0, p->getMySet());
		}
		lblPotTotal.SetLabel(Format(t_("Total: $%d"), total));
		lblPotBets.SetLabel(Format(t_("Round: $%d"), round));
		String phase = "Pre-Flop";
		switch (m_game->getCurrentHand()->getCurrentRound()) {
			case GAME_STATE_FLOP: phase = "Flop"; break;
			case GAME_STATE_TURN: phase = "Turn"; break;
			case GAME_STATE_RIVER: phase = "River"; break;
			case GAME_STATE_POST_RIVER: phase = "Showdown"; break;
			default: break;
		}
		lblTurnTitle.SetLabel(phase);
		lblGameInfo.SetLabel(Format("Game %d", m_game->getMyGameID()));
		lblHandInfo.SetLabel(Format("Hand %d", m_game->getCurrentHandID()));
		UpdateProbabilities();
	}
	if (m_imageMode)
		Refresh();
}

void GameTable::refreshSet()
{
	for (int i = 0; i < 10; i++) refreshGroupbox(i, 0);
	if (m_imageMode)
		Refresh();
}

void GameTable::nextPlayerAnimation()
{
	if (m_isPaused) {
		SetTimeCallback(200, callback(this, &GameTable::nextPlayerAnimation));
		return;
	}
	if (m_game) {
		int delay = GetActionDelayMs(); // 0 to 2 seconds delay
		if (delay > 0)
			SetTimeCallback(delay, callback(this, &GameTable::OnNextPlayer));
		else
			PostCallback(callback(this, &GameTable::OnNextPlayer));
	}
}

void GameTable::OnNextPlayer()
{
	if (!m_game)
		return;
	auto hand = m_game->getCurrentHand();
	if (!hand)
		return;
	auto bero = hand->getCurrentBeRo();
	if (bero)
		bero->nextPlayer();
}

void GameTable::flipHolecardsAllIn() {}
static String CardIdToString(int card)
{
	if (card < 0 || card >= 52) return "";
	static const char* rnk[] = { "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A" };
	static const char* sut[] = { "c", "d", "h", "s" };
	return String(rnk[card % 13]) + sut[card / 13];
}

void GameTable::logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5)
{
	boardCards[0] = card1;
	boardCards[1] = card2;
	boardCards[2] = card3;
	boardCards[3] = card4;
	boardCards[4] = card5;
	board.Refresh();
	
	String cardsStr = "";
	if (card1 >= 0) {
		cardsStr = " [" + CardIdToString(card1);
		if (card2 >= 0) cardsStr += " " + CardIdToString(card2);
		if (card3 >= 0) cardsStr += " " + CardIdToString(card3);
		if (card4 >= 0) cardsStr += " " + CardIdToString(card4);
		if (card5 >= 0) cardsStr += " " + CardIdToString(card5);
		cardsStr += "]";
	}
	
	String rName = "Preflop";
	switch(roundID) {
		case GAME_STATE_FLOP: rName = "Flop"; break;
		case GAME_STATE_TURN: rName = "Turn"; break;
		case GAME_STATE_RIVER: rName = "River"; break;
	}
	lblTurnTitle.SetLabel(rName);
	if (m_game) {
		lblGameInfo.SetLabel(Format("Game %d", m_game->getMyGameID()));
		lblHandInfo.SetLabel(Format("Hand %d", m_game->getCurrentHandID()));
	}
	AppendLog("*** " + ToUpper(rName) + " ***" + cardsStr);
}

void GameTable::refreshGroupbox(int playerID, int type)
{
	if (playerID >= 0 && playerID < 10 && m_game) {
		auto player = m_game->getPlayerByNumber(playerID);
		if (player && player->getMyActiveStatus()) {
			players[playerID].label_PlayerName.Show();
			players[playerID].textLabel_Cash.Show();
			players[playerID].textLabel_Set.Show();
			players[playerID].label_Avatar.Show();
			players[playerID].textLabel_Button.Show();
			players[playerID].actionPic.Show();
			players[playerID].label_PlayerName.SetLabel(player->getMyName());
			players[playerID].textLabel_Cash.SetLabel(Format("$%d", player->getMyCash()));
			
			int pSet = player->getMySet();
			if (pSet > 0)
				players[playerID].textLabel_Set.SetLabel(Format("$%d", pSet));
			else
				players[playerID].textLabel_Set.SetLabel("");
			
			PlayerAction act = player->getMyAction();
			
			bool isWinner = false;
			if (m_game && m_game->getCurrentHand() && m_game->getCurrentHand()->getBoard()) {
				const Vector<unsigned>& winners = m_game->getCurrentHand()->getBoard()->getWinners();
				for(unsigned id : winners) {
					if (id == player->getMyUniqueID()) {
						isWinner = true;
						break;
					}
				}
			}
			players[playerID].bg.isWinner = isWinner;
			players[playerID].bg.Refresh();

			Image aPic;
			if (isWinner) aPic = actionPics[9];
			else if ((int)act >= 0 && (int)act < 10) aPic = actionPics[(int)act];
			players[playerID].actionPic.SetImage(aPic);
			
			int h1, h2; player->getMyCards(h1, h2);
			if (h1 != -2) {
				bool showdown = (m_game->getCurrentHand() && m_game->getCurrentHand()->getCurrentRound() == GAME_STATE_POST_RIVER);
				bool folded = (player->getMyAction() == PLAYER_ACTION_FOLD);
				
				if (playerID == 0 || m_game->isGameOver() || (showdown && !folded)) {
					if (folded) {
						ImageDraw iw(72, 76);
						iw.Alpha().DrawRect(0, 0, 72, 76, RGBAZero());
						iw.DrawImage(0, 0, GetCardImage(h1));
						iw.DrawImage(26, 0, GetCardImage(h2));
						players[playerID].pixmapLabel_cards.SetImage(ApplyInterlacedTransparency(iw));
						players[playerID].pixmapLabel_cards.Show();
						players[playerID].pixmapLabel_carda.Hide();
						players[playerID].pixmapLabel_cardb.Hide();
					} else {
						players[playerID].pixmapLabel_carda.SetImage(GetCardImage(h1));
						players[playerID].pixmapLabel_cardb.SetImage(GetCardImage(h2));
						players[playerID].pixmapLabel_carda.Show();
						players[playerID].pixmapLabel_cardb.Show();
						players[playerID].pixmapLabel_cards.Hide();
					}
				} else {
					if (folded) {
						ImageDraw iw(72, 76);
						iw.Alpha().DrawRect(0, 0, 72, 76, RGBAZero());
						iw.DrawImage(0, 0, GetCardImage(-1));
						iw.DrawImage(26, 0, GetCardImage(-1));
						players[playerID].pixmapLabel_cards.SetImage(ApplyInterlacedTransparency(iw));
						players[playerID].pixmapLabel_cards.Show();
						players[playerID].pixmapLabel_carda.Hide();
						players[playerID].pixmapLabel_cardb.Hide();
					} else {
						players[playerID].pixmapLabel_carda.SetImage(GetCardImage(-1));
						players[playerID].pixmapLabel_cardb.SetImage(GetCardImage(-1));
						players[playerID].pixmapLabel_carda.Show();
						players[playerID].pixmapLabel_cardb.Show();
						players[playerID].pixmapLabel_cards.Hide();
					}
				}
			} else {
				players[playerID].pixmapLabel_carda.SetImage(Null);
				players[playerID].pixmapLabel_cardb.SetImage(Null);
				players[playerID].pixmapLabel_cards.SetImage(Null);
			}

			String avatarFile = player->getMyAvatar();
			if (!avatarFile.IsEmpty()) {
				players[playerID].label_Avatar.SetImage(StreamRaster::LoadFileAny(avatarFile));
			} else {
				if (!avatarDefault.IsEmpty()) players[playerID].label_Avatar.SetImage(avatarDefault);
				else players[playerID].label_Avatar.SetImage(CtrlImg::exclamation());
			}
			
			String dataDir = Tools::GetDataDir();
			if (player->getMyButton() == GBUTTON_DEALER) {
				if (!puckDealer.IsEmpty()) players[playerID].textLabel_Button.SetImage(puckDealer);
				else players[playerID].textLabel_Button.SetImage(StreamRaster::LoadFileAny(AppendFileName(dataDir, "gfx/dealer.png")));
			} else if (player->getMyButton() == GBUTTON_SMALL_BLIND) {
				if (!puckSB.IsEmpty()) players[playerID].textLabel_Button.SetImage(puckSB);
				else players[playerID].textLabel_Button.SetImage(StreamRaster::LoadFileAny(AppendFileName(dataDir, "gfx/small_blind.png")));
			} else if (player->getMyButton() == GBUTTON_BIG_BLIND) {
				if (!puckBB.IsEmpty()) players[playerID].textLabel_Button.SetImage(puckBB);
				else players[playerID].textLabel_Button.SetImage(StreamRaster::LoadFileAny(AppendFileName(dataDir, "gfx/big_blind.png")));
			} else {
				players[playerID].textLabel_Button.SetImage(Null);
			}

		} else {
			players[playerID].label_PlayerName.SetLabel("");
			players[playerID].textLabel_Cash.SetLabel("");
			players[playerID].textLabel_Set.SetLabel("");
			players[playerID].label_PlayerName.Hide();
			players[playerID].textLabel_Cash.Hide();
			players[playerID].textLabel_Set.Hide();
			players[playerID].bg.isWinner = false;
			players[playerID].bg.Refresh();
			players[playerID].pixmapLabel_carda.SetImage(Null);
			players[playerID].pixmapLabel_cardb.SetImage(Null);
			players[playerID].pixmapLabel_cards.SetImage(Null);
			players[playerID].pixmapLabel_carda.Hide();
			players[playerID].pixmapLabel_cardb.Hide();
			players[playerID].pixmapLabel_cards.Hide();
			players[playerID].label_Avatar.SetImage(Null);
			players[playerID].label_Avatar.Hide();
			players[playerID].textLabel_Button.SetImage(Null);
			players[playerID].textLabel_Button.Hide();
			players[playerID].actionPic.SetImage(Null);
			players[playerID].actionPic.Hide();
		}
	}
	if (m_imageMode)
		Refresh();
}

void GameTable::preflopAnimation1() {}
void GameTable::flopAnimation1() {}
void GameTable::turnAnimation1() {}
void GameTable::riverAnimation1() {}
void GameTable::postRiverAnimation1()
{
	if (m_isPaused) {
		SetTimeCallback(500, callback(this, &GameTable::postRiverAnimation1));
		return;
	}
	int speed = (int)sliderSpeed.GetData();
	int delay = (10 - speed) * 500 + 2000; // Delay to show winners
	SetTimeCallback(delay, callback(this, &GameTable::OnNextHand));
}

void GameTable::OnNextHand()
{
	if (m_game) {
		if (m_game->isGameOver()) {
			PromptOK(t_("Game Over!"));
			return;
		}
		m_game->initHand();
		m_game->startHand();
	}
}

void GameTable::logPlayerActionMsg(String playName, int action, int setValue) {
	String actStr;
	switch(action) {
		case PLAYER_ACTION_FOLD: actStr = "folds."; break;
		case PLAYER_ACTION_CALL: actStr = Format("calls $%d.", setValue); break;
		case PLAYER_ACTION_CHECK: actStr = "checks."; break;
		case PLAYER_ACTION_BET: actStr = Format("bets $%d.", setValue); break;
		case PLAYER_ACTION_RAISE: actStr = Format("raises to $%d.", setValue); break;
		case PLAYER_ACTION_ALLIN: actStr = "is all-in."; break;
		case PLAYER_ACTION_SMALL_BLIND: actStr = Format("posts small blind ($%d)", setValue); break;
		case PLAYER_ACTION_BIG_BLIND: actStr = Format("posts big blind ($%d)", setValue); break;
		default: actStr = AsString(action); break;
	}
	AppendLog("[" + playName + "] " + actStr);
}

void GameTable::logFlipHoleCardsMsg(String playerName, int card1, int card2, int cardsValueInt, String showHas)
{
	String handName = CardsValue::determineHandName(cardsValueInt, nullptr);
	AppendLog("[" + playerName + "] " + showHas + " " + handName);
}

void GameTable::logWinningHandMsg(String playerName, String handName, int amount)
{
	AppendLog(Format("[%s] wins $%d with %s", playerName, amount, handName));
}

void GameTable::UpdateProbabilities()
{
	for(int i = 0; i < probCtrl.items.GetCount(); i++) {
		if (i == 0) probCtrl.items[i].prob = 0.0; // Royal Flush 0%
		else probCtrl.items[i].prob = (double)Random(50) / 100.0;
	}
	probCtrl.Refresh();
}

void GameTable::dealBeRoCards(TexasRound state) {}
void GameTable::beRoAnimation2(TexasRound state) {}

void GameTable::meInAction()
{
	if (m_imageMode) {
		humanButtons.Hide();
		return;
	}
	humanButtons.Show();
	
	if (m_game && m_game->getCurrentHand()) {
		auto p = m_game->getPlayerByNumber(0);
		auto bero = m_game->getCurrentHand()->getCurrentBeRo();
		if (!p || !bero)
			return;
		
		int highestSet = bero->getHighestSet();
		int mySet = p->getMySet();
		int callValue = highestSet - mySet;
		int minRaise = bero->getMinimumRaise();
		
		int minBet = callValue + minRaise;
		int maxBet = p->getMyCash();
		
		if (minBet > maxBet) minBet = maxBet;
		// SliderCtrl crashes on zero span (max == min) on some paths/backends.
		// Keep a non-zero slider range and disable raise controls when no raise is possible.
		int sliderMin = minBet;
		int sliderMax = max(maxBet, minBet + 1);
		sliderBet.MinMax(sliderMin, sliderMax);
		sliderBet.SetData(minBet);
		editBet.MinMax(minBet, maxBet);
		editBet.SetData(minBet);
		bool canRaise = maxBet > minBet;
		sliderBet.Enable(canRaise);
		editBet.Enable(canRaise);
		btnBetRaise.Enable(canRaise);
		btnPot33.Enable(canRaise);
		btnPot50.Enable(canRaise);
		btnPot100.Enable(canRaise);
		
		UpdateActionLabels();
	}
}

void GameTable::postRiverRunAnimation1() {}
void GameTable::refreshCash()
{
	for (int i = 0; i < 10; i++) refreshGroupbox(i, 0);
}
void GameTable::refreshAction(int playerID, int action)
{
	refreshGroupbox(playerID, 0);
}
void GameTable::SignalNetClientError(int errorId, int osErrorCode)
{
	PromptOK(Format("Network Error: %d (OS: %d)", errorId, osErrorCode));
}

void GameTable::SetRemoteState(const ScreenGameState& state)
{
	if (!state.found) return;
	
	lblPotTotal.SetLabel(Format(t_("Total: $%d"), state.pot));
	
	for (int i = 0; i < 10; i++) {
		if (i < state.stacks.GetCount()) {
			players[i].textLabel_Cash.SetLabel(Format("$%d", state.stacks[i]));
		}
	}
	
	// Update board cards
	for (int i = 0; i < 5; i++) {
		boardCards[i] = (i < state.community_cards.GetCount()) ? state.community_cards[i] : -2;
	}
	
	Refresh();
}

END_UPP_NAMESPACE
