#ifndef _CardEngine_StartWindow_h_
#define _CardEngine_StartWindow_h_

#include <CtrlLib/CtrlLib.h>
#include <Form/Form.hpp>
#include <memory>

NAMESPACE_UPP

class StartWindow : public TopWindow {
public:
	typedef StartWindow CLASSNAME;
	StartWindow();

	void Init(class ConfigFile& config, std::shared_ptr<class EngineLog> engineLog) {
		m_config = &config;
		m_engineLog = engineLog;
	}

private:
	void HandleUiSignal(const String& path, const String& op, const String& action);
	void FillProviders();
	void UpdateTitleFromProvider();
	String SelectedProvider() const;
	void OnLocalGame();
	void OnInternetGame();
	void OnNetworkCreate();
	void OnNetworkJoin();
	void OnInfo();
	void OnSettings();
	void OnLog();
	void OnQuit();

	Form ui;
	DropList* providerChoice = nullptr;
	class ConfigFile* m_config;
	std::shared_ptr<class ServerManager> m_serverManager;
	std::shared_ptr<class EngineLog> m_engineLog;
	String m_provider;
};
END_UPP_NAMESPACE

#endif
