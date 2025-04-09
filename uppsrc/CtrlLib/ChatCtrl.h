class ChatCtrl : public PageCtrl {
	
public:
	struct Message : Pte<Message> {
		String user, qtf;
		Image icon;
		RichTextCtrl textctrl;
		
		Message& SetUser(String s);
		Message& SetText(String qtf);
		Message& SetImage(Image img);
	};
	
private:
	Array<Message> messages;
	
public:
	typedef ChatCtrl CLASSNAME;
	ChatCtrl();
	
	Message& AddMessage();
	Message& AddMessage(String user, String qtf);
	Message& AddMessage(String user, Image icon, String qtf);
	
};
