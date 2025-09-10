#ifdef HAVE_GTKMM
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/label.h>

void Gtkmm01(int test_num) {
	if (test_num == 0 || test_num < 0) {
		int argc = 1;
		char arg0[] = "gtkmmapp";
		char* argv[] = { arg0 };
		auto app = Gtk::Application::create(argc, argv, "org.example.helloworld");
		Gtk::Window window;
		window.set_title("Hello World program");
		window.set_default_size(320, 240);
		Gtk::Label label("Hello world!");
		label.set_hexpand(true);
		label.set_vexpand(true);
#if GTKMM_CHECK_VERSION(3,0,0)
		label.set_justify(Gtk::JUSTIFY_CENTER);
#endif
		window.add(label);
		window.show_all();
		app->run(window);
	}
}
#else
void Gtkmm01(int) {}
#endif

