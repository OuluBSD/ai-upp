#ifndef _CardEngine_StartWindow_h_
#define _CardEngine_StartWindow_h_

#include <CtrlLib/CtrlLib.h>
#include <memory>

NAMESPACE_UPP

#define LAYOUTFILE <TexasHoldem/StartWindow.lay>
#include <CtrlCore/lay.h>

class StartWindow : public WithStartWindowLayout<TopWindow> {
public:
	typedef StartWindow CLASSNAME;
	StartWindow();
	
	void Init(class ConfigFile& config, std::shared_ptr<class EngineLog> engineLog) {
		m_config = &config;
		m_engineLog = engineLog;
	}

private:
	void MainMenu(Bar& menu);
	void AppMenu(Bar& menu);
	void NetworkSubMenu(Bar& menu);
	void SettingsMenu(Bar& menu);

	void OnLocalGame();
	void OnInternetGame();
	void OnNetworkCreate();
	void OnNetworkJoin();
	void OnInfo();
	void OnSettings();
	void OnLog();
	void OnQuit();
	
	MenuBar  menu;
	class ConfigFile* m_config;
	std::shared_ptr<class ServerManager> m_serverManager;
	std::shared_ptr<class EngineLog> m_engineLog;
};
END_UPP_NAMESPACE

#endif
