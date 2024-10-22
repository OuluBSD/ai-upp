#include "AI.h"

NAMESPACE_UPP

void AiAnnotationItem::Jsonize(JsonIO& json)
{
	json("a", (AnnotationItem&)*this)("c", comments);
}

void AiAnnotationItem::Comment::Jsonize(JsonIO& json)
{
	json("l", rel_line)("h", (int64&)line_hash)("s", txt);
}

void AiFileInfo::Jsonize(JsonIO& json) { json("items", ai_items); }

void AiFileInfo::UpdateLinks(FileAnnotation& ann)
{
	int c0 = ann.items.GetCount();
	int c1 = ai_items.GetCount();
	if(!c1) {
		ai_items.SetCount(c0);
		for(int i = 0; i < c0; i++) {
			AnnotationItem& it0 = ann.items[i];
			AiAnnotationItem& it1 = ai_items[i];
			(AnnotationItem&)it1 = ann.items[i];
			// it1.linked = &it0;
		}
	}
	else {
		Vector<bool> linked0, linked1;
		linked0.SetCount(c0, false);
		linked1.SetCount(c1, false);
		int nonlinked0 = c0;
		int nonlinked1 = c1;

		// for (auto& it : ai_items)
		//	it.linked = 0;

		for(int tries = 0; tries < 2; tries++) {
			for(int i = 0; i < ann.items.GetCount(); i++) {
				if(linked0[i])
					continue;
				AnnotationItem& it0 = ann.items[i];
				for(int j = 0; j < ai_items.GetCount(); j++) {
					if(linked1[j])
						continue;
					AiAnnotationItem& it1 = ai_items[j];
					if((tries == 0 && it0.IsSameContent((AnnotationItem&)it1)) ||
					   (tries == 1 && it0.IsLineAreaPartialMatch((AnnotationItem&)it1))) {
						// it1.linked = &it0;
						linked0[i] = true;
						linked1[j] = true;
						nonlinked0--;
						nonlinked1--;
						break;
					}
				}
			}
		}

		for(int i = 0; i < ann.items.GetCount(); i++) {
			if(linked0[i])
				continue;
			AnnotationItem& it0 = ann.items[i];
			AiAnnotationItem& it1 = ai_items.Add();
			// it1.linked = &it0;
			(AnnotationItem&)it1 = it0;
			nonlinked0--;
		}
	}
}

void AiAnnotationItem::RemoveCommentLine(int rel_line)
{
	Vector<int> rm_list;
	for(int i = 0; i < comments.GetCount(); i++) {
		if(comments[i].rel_line == rel_line)
			rm_list << i;
	}
	comments.Remove(rm_list);
}

AiAnnotationItem::Comment* AiAnnotationItem::FindComment(int rel_line)
{
	for(Comment& c : comments)
		if(c.rel_line == rel_line)
			return &c;
	return 0;
}

END_UPP_NAMESPACE
