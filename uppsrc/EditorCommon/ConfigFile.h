#ifndef _CardEngine_ConfigFile_h_
#define _CardEngine_ConfigFile_h_

#include <Core/Core.h>

NAMESPACE_UPP

enum ConfigState { NONEXISTING, OLD, OK };
enum ConfigType { CONFIG_TYPE_INT, CONFIG_TYPE_STRING, CONFIG_TYPE_INT_LIST, CONFIG_TYPE_STRING_LIST };

struct ConfigInfo {
	ConfigInfo() : type(CONFIG_TYPE_INT) {}
	ConfigInfo(const String &n, ConfigType t, const String &d, Vector<String> l = Vector<String>()) : name(n), type(t), defaultValue(d), defaultListValue(pick(l)) {}
	String name;
	ConfigType type;
	String defaultValue;
	Vector<String> defaultListValue;
};

class ConfigFile
{
public:
	ConfigFile(char *argv0, bool readonly);
	~ConfigFile();

	void fillBuffer();
	void clearBuffer();
	void checkAndCorrectBuffer();
	void writeBuffer() const;

	void updateConfig(ConfigState);
	ConfigState getConfigState() const;

	String readConfigString(String varName) const;
	Vector<String> readConfigStringList(String varName) const;
	void writeConfigString(String varName, String varCont);
	void writeConfigStringList(String varName, const Vector<String>& varCont);
	int readConfigInt(String varName) const;
	Vector<int> readConfigIntList(String varName) const;
	void writeConfigInt(String varName, int varCont);
	void writeConfigIntList(String varName, const Vector<int>& varCont);
	void deleteConfigFile();
	
	String configFileName;

protected:
	void checkAndCorrectPlayerNames();

private:
	mutable Mutex m_configMutex;
	VectorMap<String, String> m_values;
	Vector<ConfigInfo> configList;
	Vector<ConfigInfo> configBufferList;

	String logDir;
	String dataDir;
	String cacheDir;
	String defaultGameTableStyle;
	String defaultCardDeck;

	int configRev;
	bool noWriteAccess;
	String logOnOffDefault;
	ConfigState myConfigState;
	char *myArgv0;
};

END_UPP_NAMESPACE

#endif
