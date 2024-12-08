#include "Ctrl.h"

NAMESPACE_UPP


BiographyElementsCtrl::BiographyElementsCtrl() {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << categories << vsplit;
	hsplit.SetPos(1500, 0);
	hsplit.SetPos(3000, 1);
	
	vsplit.Vert() << elements << block;
	vsplit.SetPos(3333);
	
	CtrlLayout(block);
	
	categories.AddColumn(t_("Category"));
	categories.AddColumn(t_("Entries"));
	categories.AddIndex("IDX");
	categories.Set(0, 0, "All");
	categories.Set(0, "IDX", -1);
	for(int i = 0; i < BIOCATEGORY_COUNT; i++) {
		categories.Set(i+1, 0, GetBiographyCategoryKey(i));
		categories.Set(i+1, "IDX", i);
	}
	categories.ColumnWidths("5 1");
	categories.SetSortColumn(0);
	categories.SetCursor(0);
	categories <<= THISBACK(DataCategory);
	
	elements.AddColumn(t_("Year"));
	elements.AddColumn(t_("Element"));
	elements.AddColumn(t_("Description"));
	for(int i = 0; i < SCORE_COUNT; i++)
		elements.AddColumn(ScoreTitles[i]);
	elements.AddColumn(t_("Score"));
	elements.AddIndex("CAT");
	elements.AddIndex("YEAR");
	elements.ColumnWidths("1 3 8 1 1 1 1 1 1 1 1 1 1 1");
	elements <<= THISBACK(DataElement);
	sort_column = 3 + SCORE_COUNT;
	
	block.keywords <<= THISBACK(OnValueChange);
	block.native_text <<= THISBACK(OnValueChange);
	block.text <<= THISBACK(OnValueChange);
	
	block.elements.AddColumn("Key");
	block.elements.AddColumn("Value");
	block.elements.AddColumn("Score");
	block.elements.ColumnWidths("2 12 1");
	
	
}

void BiographyElementsCtrl::Data() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography) {
		for(int i = 0; i < categories.GetCount(); i++)
			categories.Set(i, 1, 0);
		return;
	}
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	
	for(int i = 0; i < categories.GetCount(); i++) {
		int cat_i = categories.Get(i, "IDX");
		BiographyCategory& bcat = biography.GetAdd(owner, cat_i);
		categories.Set(i, 1, bcat.GetFilledCount());
	}
	
	DataCategory();
}

void BiographyElementsCtrl::DataCategory() {
	MetaDatabase& mdb = MetaDatabase::Single();
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !categories.IsCursor()) {
		elements.Clear();
		return;
	}
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	int cat_i = categories.Get("IDX");
	int row = 0;
	
	// Dump elements
	if (0) {
		LOG("BEGIN");
		Index<String> elements;
		for (const auto& a : biography.AllCategories()) {
			for (const auto& by : a.years) {
				for (const auto& el : by.elements) {
					elements.FindAdd(el.key);
				}
			}
		}
		SortIndex(elements, StdLess<String>());
		for(int i = 0; i < elements.GetCount(); i++) {
			LOG("v.Add(\"" + elements[i] + "\");");
		}
	}
	
	for (int cat_iter = 0; cat_iter < BIOCATEGORY_COUNT; cat_iter++) {
		if (cat_i >= 0 && cat_iter != cat_i)
			continue;
		
		BiographyCategory& bcat = biography.GetAdd(owner, cat_iter);
		
		for(int i = 0; i < bcat.years.GetCount(); i++) {
			const BioYear& by = bcat.years[i];
			
			for(int j = 0; j < by.elements.GetCount(); j++) {
				const BioYear::Element& el = by.elements[j];
				
				elements.Set(row, 0, by.year);
				elements.Set(row, 1, el.key);
				elements.Set(row, 2, el.value);
				elements.Set(row, "CAT", cat_iter);
				elements.Set(row, "YEAR", i);
				
				double sum = 0;
				for(int k = 0; k < SCORE_COUNT; k++) {
					elements.Set(row, 3+k, el.scores[k]);
					sum += el.scores[k];
				}
				if (sum == 0.0)
					continue;
				double av = sum / SCORE_COUNT;
				if (av > 10.0)
					av = 0;
				
				elements.Set(row, 3+SCORE_COUNT, av);
				row++;
			}
		}
	}
	
	INHIBIT_CURSOR(elements);
	elements.SetCount(row);
	elements.SetSortColumn(sort_column, true);
	if (elements.GetCount() && !elements.IsCursor())
		elements.SetCursor(0);
	
	DataElement();
}

void BiographyElementsCtrl::DataElement() {
	MetaPtrs& mp = MetaPtrs::Single();
	if (!mp.owner || !mp.biography || !elements.IsCursor())
		return;
	int cat_i = elements.Get("CAT");
	int year_i = elements.Get("YEAR");
	
	Biography& biography = *mp.biography;
	BiographyCategory& bcat = biography.GetAdd(*mp.owner, cat_i);
	const BioYear& by = bcat.years[year_i];
	
	block.year.SetData(by.year);
	block.keywords.SetData(by.keywords);
	block.native_text.SetData(by.native_text);
	block.text.SetData(by.text);
	
	for(int i = 0; i < by.elements.GetCount(); i++) {
		block.elements.Set(i, 0, Capitalize(by.elements[i].key));
		block.elements.Set(i, 1, by.elements[i].value);
		block.elements.Set(i, 2, by.elements[i].GetAverageScore());
	}
	block.elements.SetCount(by.elements.GetCount());
	block.elements.SetSortColumn(2, true);
	
}

void BiographyElementsCtrl::OnValueChange() {
	
}

void BiographyElementsCtrl::ToolMenu(Bar& bar) {
	ToolAppCtrl::ToolMenu(bar);
}


END_UPP_NAMESPACE
