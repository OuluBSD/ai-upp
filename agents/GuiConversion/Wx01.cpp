#ifdef HAVE_WX
#include <wx/wx.h>
#include <wx/app.h>

class HelloWxApp : public wxApp {
public:
    bool OnInit() override { return true; }
};

wxIMPLEMENT_APP_NO_MAIN(HelloWxApp);

void Wx01(int test_num) {
    if (test_num == 0 || test_num < 0) {
        int argc = 1;
        char arg0[] = "wxapp";
        char* argv[] = { arg0 };
        if (!wxEntryStart(argc, argv))
            return;
        if (!wxTheApp || !wxTheApp->CallOnInit()) { wxEntryCleanup(); return; }

        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Hello World program",
                                      wxDefaultPosition, wxSize(320, 240));
        wxPanel* panel = new wxPanel(frame);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        auto* label = new wxStaticText(panel, wxID_ANY, "Hello world!");
        sizer->AddStretchSpacer(1);
        sizer->Add(label, 0, wxALIGN_CENTER | wxALL, 0);
        sizer->AddStretchSpacer(1);
        panel->SetSizerAndFit(sizer);
        frame->Centre();
        frame->Show(true);
        wxTheApp->SetTopWindow(frame);

        wxTheApp->OnRun();

        frame->Destroy();
        wxTheApp->OnExit();
        wxEntryCleanup();
    }
}
#else
void Wx01(int) {}
#endif
