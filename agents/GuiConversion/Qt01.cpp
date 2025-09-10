#ifdef HAVE_QT
#include <QApplication>
#include <QLabel>

void Qt01(int test_num) {
	if (test_num == 0 || test_num < 0) {
		int argc = 1;
		char arg0[] = "qtapp";
		char* argv[] = { arg0 };
		QApplication app(argc, argv);
		QLabel lbl("Hello world!");
		lbl.setAlignment(Qt::AlignCenter);
		lbl.setWindowTitle("Hello World program");
		lbl.resize(320, 240);
		lbl.show();
		app.exec();
	}
}
#else
void Qt01(int) {}
#endif

