#include "CardBoardEditor.h"

NAMESPACE_UPP

CardBoardTreePanel::CardBoardTreePanel()
{
	Title("Board Tree").SizeHint(Size(280, 360));
	Add(tree_.SizePos());
	tree_.NoRoot();
}

void CardBoardTreePanel::SetDocument(const CardBoardDocument& document)
{
	tree_.Clear();
	for(const CardBoardElement& element : document.elements)
		AddElement(0, element);
}

void CardBoardTreePanel::AddElement(int parent, const CardBoardElement& element)
{
	String text = Format("%s  %s", CardBoardElementTypeName(element.type), element.id);
	int id = parent ? tree_.Add(parent, CtrlImg::Dir(), text) : tree_.Add(0, CtrlImg::Dir(), text);
	for(const CardBoardElement& child : element.children)
		AddElement(id, child);
	tree_.Open(id);
}

CardBoardPropertiesPanel::CardBoardPropertiesPanel()
{
	Title("Properties").SizeHint(Size(320, 360));
	properties_.AddColumn("Property");
	properties_.AddColumn("Value");
	Add(properties_.SizePos());
}

void CardBoardPropertiesPanel::SetDocument(const CardBoardDocument& document)
{
	properties_.Clear();
	properties_.Add("Provider", document.provider_id);
	properties_.Add("Game", document.game_family);
	properties_.Add("Design", Format("%d`x%d", document.design_size.cx, document.design_size.cy));
	properties_.Add("Root elements", document.elements.GetCount());
	String error = document.Validate();
	properties_.Add("Validation", error.IsEmpty() ? "ok" : error);
}

CardBoardDiagnosticsPanel::CardBoardDiagnosticsPanel()
{
	Title("Diagnostics").SizeHint(Size(640, 180));
	text_.SetReadOnly();
	Add(text_.SizePos());
}

void CardBoardDiagnosticsPanel::SetText(const String& text)
{
	text_.Set(text);
}

END_UPP_NAMESPACE
