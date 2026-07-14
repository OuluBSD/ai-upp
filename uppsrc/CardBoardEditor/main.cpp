#include "CardBoardEditor.h"

using namespace Upp;

GUI_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(RunCardBoardEditorCli(args))
		return;
	CardBoardEditorWindow window;
	window.Run();
}
