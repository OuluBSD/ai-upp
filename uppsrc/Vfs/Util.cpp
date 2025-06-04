#include "Vfs.h"

NAMESPACE_UPP





void PathIdentifier::Clear() {
	memset(this, 0, sizeof(PathIdentifier));
}

String PathIdentifier::ToString() const {
	if (!begin || begin == end)
		return String();
	String s;
	const Token* iter = begin;
	const bool* is_meta = &this->is_meta[0];
	while (iter != end) {
		if (iter->type == TK_ID) {
			if (*is_meta)
				s.Cat('$');
			is_meta++;
			s.Cat(iter->GetTextValue());
		}
		else if (iter->type == '$')
			; // pass
		else
			s.Cat(iter->GetTextValue());
		iter++;
	}
	return s;
}

bool PathIdentifier::IsEmpty() const {
	return part_count == 0;
}

bool PathIdentifier::HasMeta() const {
	for(int i = 0; i < part_count; i++)
		if (is_meta[i])
			return true;
	return false;
}

bool PathIdentifier::HasPartialMeta() const {
	for(int i = 0; i < part_count; i++)
		if (is_meta[i])
			return i > 0; // if first is meta, then it's full, not partial
	return false;
}




bool IsTypedNode(CodeCursor src) {
	return IsPartially(src, SEMT_TYPE);
}

bool IsMetaTypedNode(CodeCursor src) {
	return IsPartially(src, SEMT_META_TYPE);
}

bool IsRvalReturn(CodeCursor src) {
	return IsPartially(src, SEMT_WITH_RVAL_RET);
}


END_UPP_NAMESPACE
