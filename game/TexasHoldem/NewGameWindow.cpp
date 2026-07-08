#include "NewGameWindow.h"
#include <EditorCommon/ConfigFile.h>

NAMESPACE_UPP

namespace {
int ValidLocalPlayers(int value)
{
	return value >= 2 && value <= 10 ? value : 10;
}

int ValidLocalCash(int value)
{
	return value >= 1000 && value <= 1000000 ? value : 2000;
}

int ValidLocalSpeed(int value)
{
	return value >= 1 && value <= 11 ? value : 4;
}
}

NewGameWindow::NewGameWindow()
{
	CtrlLayout(*this);
	Title(t_("Start Local Game"));
	
	useSavedBlinds.SetData(1);
	useSavedBlinds << [this] { if(useSavedBlinds) changeBlinds.SetData(0); };
	changeBlinds << [this] { if(changeBlinds) useSavedBlinds.SetData(0); };

	btnOK << [this] { OnOK(); };
	btnCancel << [this] { OnCancel(); };
}

void NewGameWindow::Init(class ConfigFile& config)
{
	numPlayers.SetData(ValidLocalPlayers(config.readConfigInt("LocalNumPlayers")));
	startCash.SetData(ValidLocalCash(config.readConfigInt("LocalStartCash")));
	gameSpeed.SetData(ValidLocalSpeed(config.readConfigInt("LocalGameSpeed")));
}

void NewGameWindow::OnOK()
{
	Break(IDOK);
}

void NewGameWindow::OnCancel()
{
	Break(IDCANCEL);
}

END_UPP_NAMESPACE
