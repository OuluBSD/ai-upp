#include "Local.h"

#ifdef VIRTUALGUI

#define LLOG(x) // DLOG(x)

NAMESPACE_UPP

void Ctrl::GuiPlatformConstruct()
{
}

void Ctrl::GuiPlatformRemove()
{
}

void Ctrl::GuiPlatformGetTopRect(Rect& r) const
{
}

bool Ctrl::GuiPlatformRefreshFrameSpecial(const Rect& r)
{
	return false;
}

bool Ctrl::GuiPlatformSetFullRefreshSpecial()
{
	return false;
}

String GuiPlatformGetKeyDesc(dword key)
{
	return Null;
}

void Ctrl::GuiPlatformSelection(PasteClip&)
{
}

void GuiPlatformAdjustDragImage(ImageBuffer&)
{
}

bool GuiPlatformHasSizeGrip()
{
	return true;
}

void GuiPlatformGripResize(TopWindow *q)
{
	q->GripResize();
}

Color GuiPlatformGetScreenPixel(int x, int y)
{
	return Null;
}

void GuiPlatformAfterMenuPopUp()
{
}

String Ctrl::Name() const {
	GuiLock __;
#ifdef CPU_64
	String s = String(typeid(*this).name()) + " : 0x" + FormatIntHex(this);
#else
	String s = String(typeid(*this).name()) + " : " + Format("0x%x", (int) this);
#endif
	Ctrl *parent = GetParent();
	if(parent)
		s << "(parent " << String(typeid(*parent).name()) << ")";
	return s;
}

void  Ctrl::SetMouseCursor(const Image& image)
{
	GuiLock __;
	fbCursorImage = image;
	if(VirtualGuiPtr->GetOptions() & GUI_SETMOUSECURSOR)
		VirtualGuiPtr->SetMouseCursor(image);
}

dword VirtualGui::GetOptions()
{
	return 0;
}

bool VirtualGui::WantsOwnWindowFrame()
{
	return true;
}

bool VirtualGui::WantsPerWindowRouting()
{
	return false;
}

void VirtualGui::WindowOpened(TopWindow *)
{
}

void VirtualGui::WindowClosed(TopWindow *)
{
}

void VirtualGui::SelectWindow(TopWindow *)
{
}

void VirtualGui::WindowTitleChanged(TopWindow *)
{
}

void VirtualGui::SetMouseCursor(const Image& image) {}
void VirtualGui::SetCaret(const Rect& caret) {}

END_UPP_NAMESPACE

#endif
