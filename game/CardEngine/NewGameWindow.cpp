#include "NewGameWindow.h"
#include <EditorCommon/ConfigFile.h>

NAMESPACE_UPP

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
	numPlayers.SetData(config.readConfigInt("LocalNumPlayers"));
	if (numPlayers.GetData().IsNull()) numPlayers.SetData(10);
	
	startCash.SetData(config.readConfigInt("LocalStartCash"));
	if (startCash.GetData().IsNull()) startCash.SetData(2000);
	
	gameSpeed.SetData(config.readConfigInt("LocalGameSpeed"));
	if (gameSpeed.GetData().IsNull()) gameSpeed.SetData(4);
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
