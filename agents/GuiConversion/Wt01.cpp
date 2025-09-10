#ifdef HAVE_WT
#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WDialog.h>
using namespace Wt;

// Wt applications run in a web server context.

void Wt01(int test_num) {
    if (test_num == 0 || test_num < 0) {
        int argc = 3;
        char arg0[] = "wtapp";
        char arg1[] = "--http-address=127.0.0.1";
        char arg2[] = "--http-port=8080";
        char* argv[] = { arg0, arg1, arg2 };

        auto createApp = [](const WEnvironment& env) {
            auto app = std::make_unique<WApplication>(env);
            app->setTitle("Hello World program");

            // Use a dialog as a window/frame analog and add a label
            auto dlg = app->addChild(std::make_unique<WDialog>("Hello World program"));
            dlg->contents()->addNew<WText>("Hello world!");
            dlg->show();

            return app;
        };
        WRun(argc, argv, createApp);
    }
}
#else
void Wt01(int) {}
#endif
