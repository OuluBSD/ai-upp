#pragma once
#ifndef _CtrlLib_ChatCtrl_h_
#define _CtrlLib_ChatCtrl_h_

#include "Ctrl.h"
#include <vector>
#include <functional>
#include <memory>

namespace Upp {

class ChatCtrl : public PageCtrl {
	
public:
	struct Message : Pte<Message> {
		RichTextCtrl textctrl;
		Ptr<PageCtrl::Item> item;
		Message& SetUser(String s);
		Message& SetText(String qtf);
		Message& SetImage(Image img);
	};
	
private:
	Array<Message> messages;
	
public:
	typedef ChatCtrl CLASSNAME;
	ChatCtrl();
	
	void Clear();
	int GetMessageCount() const;
	Message& GetMessage(int i);
	Message& AddMessage();
	Message& AddMessage(String user, String qtf);
	Message& AddMessage(String user, Image icon, String qtf);
	
};

}

#endif