#include <EditorCommon/ConfigFile.h>

NAMESPACE_UPP

ConfigFile::ConfigFile(char *argv0, bool readonly) : noWriteAccess(readonly), myArgv0(argv0)
{
	configFileName = "pkr.cfg"; // Default
	if (!noWriteAccess)
		fillBuffer();
}

ConfigFile::~ConfigFile()
{
	if (!noWriteAccess)
		writeBuffer();
}

void ConfigFile::fillBuffer()
{
	Mutex::Lock lock(m_configMutex);
	FileIn in(configFileName);
	if (!in) return;
	
	while (!in.IsEof()) {
		String line = in.GetLine();
		int q = line.Find('=');
		if (q >= 0) {
			String key = TrimBoth(line.Left(q));
			String val = TrimBoth(line.Mid(q + 1));
			m_values.GetAdd(key) = val;
		}
	}
}

void ConfigFile::clearBuffer()
{
	Mutex::Lock lock(m_configMutex);
	m_values.Clear();
}

void ConfigFile::writeBuffer() const
{
	if (noWriteAccess) return;
	Mutex::Lock lock(m_configMutex);
	FileOut out(configFileName);
	if (!out) return;
	
	for (int i = 0; i < m_values.GetCount(); i++) {
		out << m_values.GetKey(i) << "=" << m_values[i] << "\n";
	}
}

String ConfigFile::readConfigString(String varName) const
{
	Mutex::Lock lock(m_configMutex);
	return m_values.Get(varName, "");
}

int ConfigFile::readConfigInt(String varName) const
{
	Mutex::Lock lock(m_configMutex);
	String s = m_values.Get(varName, "");
	return s.IsEmpty() ? 0 : StrInt(s);
}

void ConfigFile::writeConfigString(String varName, String varCont)
{
	Mutex::Lock lock(m_configMutex);
	m_values.GetAdd(varName) = varCont;
}

void ConfigFile::writeConfigInt(String varName, int varCont)
{
	Mutex::Lock lock(m_configMutex);
	m_values.GetAdd(varName) = IntStr(varCont);
}

void ConfigFile::checkAndCorrectBuffer() {}
void ConfigFile::updateConfig(ConfigState) {}
ConfigState ConfigFile::getConfigState() const { return OK; }

Vector<String> ConfigFile::readConfigStringList(String varName) const { return Vector<String>(); }
void ConfigFile::writeConfigStringList(String varName, const Vector<String>& varCont) {}
Vector<int> ConfigFile::readConfigIntList(String varName) const { return Vector<int>(); }
void ConfigFile::writeConfigIntList(String varName, const Vector<int>& varCont) {}
void ConfigFile::deleteConfigFile() { DeleteFile(configFileName); }
void ConfigFile::checkAndCorrectPlayerNames() {}

END_UPP_NAMESPACE