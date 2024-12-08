#include "Ctrl.h"

NAMESPACE_UPP


LeadTemplateCtrl::LeadTemplateCtrl() {
	Add(templates.SizePos());
	
	
	templates.AddColumn(t_("Title"));
	templates.AddColumn(t_("Text"));
	templates.AddColumn(t_("Submission price"));
	templates.AddColumn(t_("Class"));
	templates.AddColumn(t_("Speciality"));
	templates.AddColumn(t_("Profit #1"));
	templates.AddColumn(t_("Profit #2"));
	templates.AddColumn(t_("Profit #3"));
	templates.AddColumn(t_("Org. #1"));
	templates.AddColumn(t_("Org. #2"));
	templates.AddColumn(t_("Org. #3"));
	templates.AddColumn(t_("Score"));
	templates.AddIndex("IDX");
	templates.ColumnWidths("3 10 1 2 2 3 3 3 3 3 3 1");
	
}

void LeadTemplateCtrl::Data() {
	MetaDatabase& mdb = MetaDatabase::Single();
	LeadDataTemplate& ldt = LeadDataTemplate::Single();
	int lng = mdb.GetLanguageIndex();
	
	int row = 0;
	for (const LeadTemplate& lt : ldt.templates) {
		templates.Set(row, 0, lt.title);
		templates.Set(row, 1, lt.text);
		templates.Set(row, 2, lt.submission_price);
		
		String cls, spec;
		if (lt.author_classes.GetCount())
			cls = ldt.author_classes[lt.author_classes[0]];
		if (lt.author_specialities.GetCount())
			spec = ldt.author_specialities[lt.author_specialities[0]];
		
		templates.Set(row, 3, cls);
		templates.Set(row, 4, spec);
		
		for(int i = 0; i < 3; i++) {
			int col = 5+i;
			String s;
			if (i < lt.profit_reasons.GetCount())
				s = ldt.profit_reasons[lt.profit_reasons[i]];
			templates.Set(row, col, s);
		}
		
		for(int i = 0; i < 3; i++) {
			int col = 8+i;
			String s;
			if (i < lt.organizational_reasons.GetCount())
				s = ldt.organizational_reasons[lt.organizational_reasons[i]];
			templates.Set(row, col, s);
		}
		
		double score = 0;
		if (lt.orig_lead_lng == lng) {
			if (lt.orig_lead_idx >= mdb.lead_data.opportunities.GetCount()) {
				LOG("error: lt.orig_lead_idx >= mdb.lead_data.opportunities.GetCount()");
			}
			else {
				const LeadOpportunity& o = mdb.lead_data.opportunities[lt.orig_lead_idx];
				score = o.average_payout_estimation;
				//lt.submission_price = o.min_entry_price_cents * 0.01;
			}
		}
		templates.Set(row, 11, score);
		
		row++;
	}
	INHIBIT_CURSOR(templates);
	templates.SetCount(row);
	templates.SetSortColumn(11, true);
	if (!templates.IsCursor() && templates.GetCount())
		templates.SetCursor(0);
}

void LeadTemplateCtrl::ToolMenu(Bar& bar) {
	ToolAppCtrl::ToolMenu(bar);
}

void LeadTemplateCtrl::Do(int fn) {
	
}


END_UPP_NAMESPACE
