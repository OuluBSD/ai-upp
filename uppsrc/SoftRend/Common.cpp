#include "SoftRend.h"

NAMESPACE_UPP

GenericFragmentShaderArgs::GenericFragmentShaderArgs() {
	for(int i = 0; i < TEXTYPE_COUNT; i++)
		color_buf[i] = 0;
}

END_UPP_NAMESPACE
