#include "CtrlLib.h"

namespace Upp {

ChatCtrl::Message& ChatCtrl::Message::SetUser(String s) {
	user = s;
	return *this;
}

ChatCtrl::Message& ChatCtrl::Message::SetText(String qtf) {
	this->qtf = qtf;
	return *this;
}

ChatCtrl::Message& ChatCtrl::Message::SetImage(Image img) {
	this->icon = img;
	return *this;
}

ChatCtrl::ChatCtrl() {
	
}

ChatCtrl::Message& ChatCtrl::AddMessage() {
	Message& msg = messages.Add();
	PageCtrl::Add(msg.textctrl, "");
	return msg;
}

ChatCtrl::Message& ChatCtrl::AddMessage(String user, String qtf) {
	Message& msg = messages.Add();
	msg.textctrl.SetQTF(qtf);
	auto& item = PageCtrl::Add(msg.textctrl, user);
	PostCallback([this,&msg,&item]{
		int h = msg.textctrl.GetCy();
		item.Height(h);
		Layout();
	});
	return msg;
}

ChatCtrl::Message& ChatCtrl::AddMessage(String user, Image icon, String qtf) {
	Message& msg = messages.Add();
	msg.textctrl.SetQTF(qtf);
	auto& item = PageCtrl::Add(msg.textctrl, icon, user);
	PostCallback([this,&msg,&item]{
		int h = msg.textctrl.GetCy();
		item.Height(h);
		Layout();
	});
	return msg;
}

}
