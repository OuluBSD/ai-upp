#include <VisualStateModel/VisualStateModel.h>

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.GetCount() == 2 && args[0] == "--shader-evidence-frame-config") {
		int rc = VsmRunShaderEvidenceFrameConfig(args[1]);
		if(rc)
			SetExitCode(rc);
		return;
	}
	Cout() << "VideoLiveRecognitionLoopConsole\n"
	       << "  --shader-evidence-frame-config <json>\n";
	if(!args.IsEmpty())
		SetExitCode(1);
}
