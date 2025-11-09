#include "TextParsing.h"

NAMESPACE_UPP

String NormalizePathSeparators(const String& path) {
	String normalized = path;
	if (normalized.Find('/') < 0 && normalized.Find('.') >= 0) {
		Vector<String> parts = Split(normalized, ".");
		bool valid = !parts.IsEmpty();
		for (const String& part : parts)
			if (part.IsEmpty())
				valid = false;
		if (valid)
			normalized = Join(parts, "/");
	}
	return normalized;
}

END_UPP_NAMESPACE