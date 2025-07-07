#include "FormEditor.h"


#define IMAGECLASS FormEditImg
#define IMAGEFILE <FormEditor/FormEdit.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP

DockableFormEdit::DockableFormEdit() {
	standalone = true;
	this->Title(t_("GUI Form Editor (c) Web\'n\'soft Group"));
	this->Sizeable().MinimizeBox().MaximizeBox();
	this->WhenTitle = [this](String s) {this->Title(s);};
	
	MultiButton::SubButton* b = &_GUILayouts.AddButton();
	b->SetImage(FormEditImg::AddGuiLayout());
	*b <<= THISBACK(AddGUILayout);
	b->Left();
	b = &_GUILayouts.AddButton();
	b->SetImage(FormEditImg::RemoveGuiLayout());
	*b <<= THISBACK(RemoveGUILayout);
	b->Left();

	b = &_GUILayouts.AddButton();
	b->Tip(t_("Lock interface..."));
	b->SetImage(FormEditImg::Locking());
	*b <<= THISBACK(ToggleLayoutLock);

	b = &_GUILayouts.AddButton();
	b->Tip(t_("Save layout"));
	b->SetImage(FormEditImg::SaveLayout());
	*b <<= THISBACK(SaveGUILayout);

	_GUILayouts.WhenAction = THISBACK(UpdateGUILayout);
	
	this->Add(_CtrlContainer.SizePos());
	this->Add(_Container.SizePos());
	
	Construct();
}

void DockableFormEdit::DockInit()
{
	WindowButtons(false, false, true);

	DockLeft(Dockable(_LayoutList, t_("Layouts")).SizeHint(Size(200, 200)));
	DockRight(Dockable(_ItemList, t_("Items")).SizeHint(Size(250, 200)));
	DockRight(Dockable(_ItemProperties, t_("Item properties")).SizeHint(Size(250, 300)));

	if (!FileExists(ConfigFile("Layouts.bin")))
		SaveLayout(t_(" Default"));
	LoadGUILayouts(true);
	UpdateTools();
	UpdateGUILayout();
}






FormEditWindow::FormEditWindow() {
	standalone = true;
	this->Title(t_("GUI Form Editor (c) Web\'n\'soft Group"));
	this->Sizeable().MinimizeBox().MaximizeBox();
	this->WhenTitle = [this](String s) {this->Title(s);};
	
	Add(hsplit.SizePos());
	hsplit.Horz(vsplit, main);
	hsplit.SetPos(2000);
	
	vsplit.Vert() << _LayoutList << _ItemList << _ItemProperties;
	
	main.Add(_CtrlContainer.SizePos());
	main.Add(_Container.SizePos());
	
	Construct();
}





FormEditCtrl::FormEditCtrl() {
	Add(hsplit.SizePos());
	hsplit.Horz(vsplit, main);
	hsplit.SetPos(2000);
	
	vsplit.Vert() << _LayoutList << _ItemList << _ItemProperties;
	
	main.Add(_CtrlContainer.SizePos());
	main.Add(_Container.SizePos());
	
	Construct();
}


END_UPP_NAMESPACE
