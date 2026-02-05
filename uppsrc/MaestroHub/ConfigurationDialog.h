#ifndef _MaestroHub_ConfigurationDialog_h_
#define _MaestroHub_ConfigurationDialog_h_

#include "MaestroHub.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class ConfigurationDialog : public WithConfigurationLayout<TopWindow> {
	EditString   value_editor;
	One<SettingsManager> sm;
	String root;

public:
	void Load(const String& maestro_root);
	void Save();
	
	typedef ConfigurationDialog CLASSNAME;
	ConfigurationDialog();
};

END_UPP_NAMESPACE

#endif
