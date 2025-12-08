#include <Core/Core.h>
#include <RichText/RichText.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
    SetCoutLog();

    const Vector<String>& cmd_line = CommandLine();
    if (cmd_line.GetCount() < 2 || cmd_line[1] == "-h" || cmd_line[1] == "--help") {
        LOG("QtfReader - Convert U++ .qtf files to plain text");
        LOG("Usage: QtfReader <input.qtf>");
        LOG("Output is written to stdout");
        return;
    }

    String filename = cmd_line[1];

    if (!FileExists(filename)) {
        LOG("File not found: " + filename);
        return;
    }

    String qtf_content = LoadFile(filename);
    if (qtf_content.IsEmpty()) {
        LOG("Could not read file: " + filename);
        return;
    }

    RichText rt = ParseQTF(qtf_content);

    // Convert RichText to plain text
    String result;
    for(int i = 0; i < rt.GetPartCount(); i++) {
        if(rt.IsPara(i)) {
            RichPara para = rt.Get(i);
            for(int j = 0; j < para.GetCount(); j++) {
                const RichPara::Part& part = para[j];
                if(part.text.GetCount() > 0) {
                    result += part.text.ToString();  // Convert WString to String using ToString()
                }
            }
            result += "\n"; // Add newline after each paragraph
        }
    }

    LOG(result);
}