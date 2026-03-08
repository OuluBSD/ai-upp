#include <EditorCommon/Tools.h>

NAMESPACE_UPP

void Tools::SeedRandom(unsigned seed)
{
	Upp::SeedRandom(seed);
}

void Tools::ShuffleArrayNonDeterministic(int *inout, int count)
{
	for (int i = 0; i < count; i++) {
		int j = Random(count);
		Swap(inout[i], inout[j]);
	}
}

void Tools::GetRand(int minValue, int maxValue, int count, int *out)
{
	int range = maxValue - minValue + 1;
	for (int i = 0; i < count; i++) {
		out[i] = Random(range) + minValue;
	}
}

String Tools::GetDataDir()
{
	static String dataDir;
	if (dataDir.IsEmpty()) {
		// Priority order for finding assets
		Vector<String> paths;
		paths.Add(GetFileDirectory(GetExeFilePath()) + "data");
		paths.Add(GetFileDirectory(GetExeFilePath()) + "share/pokerth");
		paths.Add(GetHomeDirectory() + "/.pokerth/data");
		paths.Add("/usr/share/pokerth");
		paths.Add(GetCurrentDirectory() + "/share/pokerth");
		paths.Add(GetCurrentDirectory() + "/data");

		for (const String& p : paths) {
			if (DirectoryExists(p) && FileExists(AppendFileName(p, "gfx/cards/default/flipside.png"))) {
				dataDir = p;
				break;
			}
		}
		
		// Fallback to CWD data
		if (dataDir.IsEmpty()) dataDir = GetCurrentDirectory() + "/data";
	}
	return dataDir;
}

END_UPP_NAMESPACE