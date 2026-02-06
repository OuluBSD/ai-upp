#ifndef _WebDriver_Keyboard_h_
#define _WebDriver_Keyboard_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

class Keyboard {
public:
	Keyboard();
	
	void Send_keys(const String& keys) const;
	void Send_modifier_keys(const String& modifiers) const;
	
private:
	String current_keys_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif