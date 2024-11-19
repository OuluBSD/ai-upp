#include "TextCore.h"

NAMESPACE_UPP

void AiTask::CreateInput_GetTokenData() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	
	TokenArgs args;
	args.Put(this->args[0]);
	
	
	if (args.fn == 0) {
		{
			auto& list = input.AddSub().Title("List of word classes");
			list.Add("Nouns");
			list.Add("Verbs");
			list.Add("Adjectives");
			list.Add("Adverbs");
			list.Add("Pronouns");
			list.Add("Prepositions");
			list.Add("Conjunctions");
			list.Add("Determiners");
			list.Add("Interjections");
			list.Add("Articles");
			list.Add("Modal verbs");
			list.Add("Gerunds");
			list.Add("Infinitives");
			list.Add("Participles");
			list.Add("Definite article");
			list.Add("Indefinite article");
			list.Add("Proper nouns");
			list.Add("Collective nouns");
			list.Add("Concrete nouns");
			list.Add("Abstract nouns");
			list.Add("Irregular verbs");
			list.Add("Regular verbs");
			list.Add("Transitive verbs");
			list.Add("Intransitive verbs");
			list.Add("Auxiliary verbs");
			list.Add("Reflexive verbs");
			list.Add("Imperative verbs");
			list.Add("First person pronouns");
			list.Add("Second person pronouns");
			list.Add("Third person pronouns");
			list.Add("Possessive pronouns");
			list.Add("Demonstrative pronouns");
			list.Add("Relative pronouns");
			list.Add("Intensive pronouns");
			list.Add("Indefinite pronouns");
			list.Add("Personal pronouns");
			list.Add("Subject pronouns");
			list.Add("Objective pronouns");
			list.Add("Possessive determiners");
			list.Add("Possessive adjectives");
			list.Add("Comparative adjectives");
			list.Add("Superlative adjectives");
			list.Add("Proper adjectives");
			list.Add("Positive adjectives");
			list.Add("Negative adjectives");
			list.Add("etc.");
		}
		{
			auto& list = input.AddSub().Title("List \"A\" words");
			list.NumberedLines();
			list.Add("You");
			list.Add("what's");
			list.Add("smile");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("Word classes for the list \"A\" (lowercase)");
			answer.NumberedLines();
			answer.Add("you: pronoun");
			answer.Add("what's: contraction (what + is)");
			answer.Add("smile: noun | verb");
		}
		input.response_length = 2*1024;
	}
	if (args.fn == 1) {
		{
			auto& list = input.AddSub().Title("List of word classes");
			list.Add("Nouns");
			list.Add("Verbs");
			list.Add("Adjectives");
			list.Add("Adverbs");
			list.Add("Pronouns");
			list.Add("Prepositions");
			list.Add("Conjunctions");
			list.Add("Determiners");
			list.Add("Interjections");
			list.Add("Articles");
			list.Add("Modal verbs");
			list.Add("Gerunds");
			list.Add("Infinitives");
			list.Add("Participles");
			list.Add("Definite article");
			list.Add("Indefinite article");
			list.Add("Proper nouns");
			list.Add("Collective nouns");
			list.Add("Concrete nouns");
			list.Add("Abstract nouns");
			list.Add("Irregular verbs");
			list.Add("Regular verbs");
			list.Add("Transitive verbs");
			list.Add("Intransitive verbs");
			list.Add("Auxiliary verbs");
			list.Add("Reflexive verbs");
			list.Add("Imperative verbs");
			list.Add("First person pronouns");
			list.Add("Second person pronouns");
			list.Add("Third person pronouns");
			list.Add("Possessive pronouns");
			list.Add("Demonstrative pronouns");
			list.Add("Relative pronouns");
			list.Add("Intensive pronouns");
			list.Add("Indefinite pronouns");
			list.Add("Personal pronouns");
			list.Add("Subject pronouns");
			list.Add("Objective pronouns");
			list.Add("Possessive determiners");
			list.Add("Possessive adjectives");
			list.Add("Comparative adjectives");
			list.Add("Superlative adjectives");
			list.Add("Proper adjectives");
			list.Add("Positive adjectives");
			list.Add("Negative adjectives");
			list.Add("etc.");
		}
		{
			auto& list = input.AddSub().Title("List \"A\" word pairs");
			list.NumberedLines();
			list.Add("automobile drives");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("Word classes for the list \"A\" (lowercase)");
			answer.NumberedLines();
			answer.Add("automobile drives: noun, verb");
		}
		input.response_length = 2*1024;
	}
	if (args.fn == 2) {
		{
			auto& list = input.AddSub().Title("List of sentence structures");
			list.Add("declarative sentence");
			list.Add("conditional sentence");
			list.Add("descriptive sentence");
			list.Add("causal sentence");
			list.Add("subject-verb-object sentence");
			list.Add("subject-verb-adjective sentence");
			/*list.Add("subject-verb-predicate sentence");
			list.Add("adverbial sentence");
			list.Add("compound sentence");
			list.Add("complex sentence");
			list.Add("simple sentence");
			list.Add("compound-complex sentence");
			list.Add("exclamatory sentence");
			list.Add("interrogative sentence");
			list.Add("imperative sentence");
			list.Add("parallel sentence ");
			list.Add("climax sentence");*/
			list.Add("etc.");
			/*list.Add("rhetorical question sentence");
			list.Add("antithesis sentence ");
			list.Add("repetition sentence ");
			list.Add("aposiopesis sentence ");
			list.Add("flashback sentence");
			list.Add("foreshadowing sentence ");
			list.Add("juxtaposition sentence ");
			list.Add("alliteration sentence ");
			list.Add("simile sentence ");
			list.Add("metaphor sentence ");
			list.Add("personification sentence ");
			list.Add("hyperbole sentence ");
			list.Add("litotes sentence ");
			list.Add("irony sentence ");
			list.Add("onomatopoeia sentence");
			list.Add("oxymoron sentence");
			list.Add("zeugma sentence");
			list.Add("ellipsis sentence");
			list.Add("chiasmus sentence");
			list.Add("anaphora sentence");
			list.Add("polysyndeton sentence");
			list.Add("asyndeton sentence");
			list.Add("anadiplosis sentence");
			list.Add("epistrophe sentence");
			list.Add("metonymy sentence");
			list.Add("synecdoche sentence");
			list.Add("epanalepsis sentence");
			list.Add("antanaclasis sentence");
			list.Add("syllepsis sentence");
			list.Add("anastrophe sentence");
			list.Add("polysyndeton sentence");
			list.Add("anadiplosis sentence");
			list.Add("period sentence");
			list.Add("loose sentence");
			list.Add("periodic sentence");
			list.Add("cumulative sentence");
			list.Add("unbalanced sentence");
			list.Add("balanced sentence");
			list.Add("split sentence");
			list.Add("parenthetical sentence");
			list.Add("regular sentence");
			list.Add("irregular sentence");
			list.Add("declarative-sentence");
			list.Add("rhetorical sentence");
			list.Add("compound-complex sentence");
			list.Add("antithetic sentence");
			list.Add("sentential sentence");
			list.Add("subordinate sentence");
			list.Add("attributive sentence");
			list.Add("predicative sentence");*/
		}
		{
			auto& list = input.AddSub().Title("List of classes of sentences");
			list.Add("independent clause");
			list.Add("dependent clause ");
			list.Add("coordinating clause ");
			list.Add("modifying clause ");
			list.Add("non-coordinating clause");
			list.Add("subordinating clause ");
			/*list.Add("narrator clause");
			list.Add("subject pronoun clause");
			list.Add("object pronoun clause ");
			list.Add("relative clause ");
			list.Add("attributive adjective clause ");
			list.Add("predicative adjective clause");
			list.Add("narrative verb clause ");
			list.Add("coordinating conjunction clause ");
			list.Add("deciding conjunction clause");
			list.Add("comparative conjunction clause");
			list.Add("conditional conjunction clause");
			list.Add("descriptive conjunction clause");
			list.Add("correlatives conjunction clause");
			list.Add("time conjunction clause");
			list.Add("reason conjunction clause");
			list.Add("place conjunction clause");
			list.Add("manner conjunction clause");
			list.Add("intrinsic conjunction clause");*/
			list.Add("etc.");
			/*
			list.Add("excessive conjunction clause");
			list.Add("restriction conjunction clause");
			list.Add("time-adverbial clause");
			list.Add("manner-adverbial clause");
			list.Add("place-adverbial clause ");
			list.Add("reason-adverbial clause");
			list.Add("object-adverbial clause");
			list.Add("predicate-adverbial clause");
			list.Add("sequential-adverbial clause");
			list.Add("causal-adverbial clause ");
			list.Add("concessive-adverbial clause ");
			list.Add("contrast-adverbial clause ");
			list.Add("purpose-adverbial clause");
			list.Add("result-adverbial clause");
			list.Add("condition-adverbial clause ");
			list.Add("supplementary-adverbial clause");
			list.Add("relativizing clause");
			list.Add("comparative relative clause");
			list.Add("subject relative clause");
			list.Add("object relative clause");
			list.Add("determinative relative clause");
			list.Add("presupposed relative clause");
			list.Add("subject-relative clause");
			list.Add("objective-relative clause");
			list.Add("descriptive-relative clause");
			list.Add("relative pronoun clause");
			list.Add("adjectival relative clause");
			list.Add("adjective noun clause");
			list.Add("dependent infinitive clause");
			list.Add("independent infinitive clause");
			list.Add("verb tense clause");
			list.Add("past tense clause");
			list.Add("present tense clause");
			list.Add("future tense clause");
			list.Add("perfect tense clause");
			list.Add("progressive tense clause");
			list.Add("intelligibly verb clause");
			list.Add("interrogative clause");
			list.Add("adverbial interrogative clause");
			list.Add("indicative verb clause ");
			list.Add("imperatively verb clause");
			list.Add("minimally verb clause");
			list.Add("neatly verb clause");
			list.Add("emphatic verb clause");
			list.Add("non existence verb clause");
			list.Add("directional verb clause");
			list.Add("determinate verb clause");
			list.Add("descriptive verb clause ");
			list.Add("tricuspid verb clause");
			list.Add("interrogative verb clause");
			list.Add("directive verb clause");
			list.Add("unconvincing verb clause");
			list.Add("parenthetical verb clause");
			list.Add("elementary clause");
			list.Add("secondary clause");
			list.Add("complex primary clause");
			list.Add("subordinate primary clause");
			list.Add("principal primary clause ");
			list.Add("secondary primary clause ");
			list.Add("independent interrogatory clause");
			list.Add("interrogative-adverb clause ");
			list.Add("preterite clause ");
			list.Add("declarative-apostrophized clause ");
			list.Add("explanatory clause ");
			list.Add("nonrestrictive(replicative) clause");
			list.Add("restrictive (restricting or defining) clause");*/
		}
		{
			auto& list = input.AddSub().Title("List of classified sentences");
			list.NumberedLines();
			/*list.Add("{noun}{verb}{adjective}");
			list.Add("{adjective}{noun}{preposition}{noun}");
			list.Add("{conjunction}{pronoun}{verb}{noun}");*/
			list.Add("noun,verb,adjective");
			list.Add("adjective,noun,preposition,noun");
			list.Add("conjunction,pronoun,verb,noun");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("List of titles of classified sentences");
			answer.NumberedLines();
			answer.Add("noun,verb,adjective: independent clause");
			answer.Add("adjective,noun,preposition,noun: prepositional sentence");
			answer.Add("conjunction,pronoun,verb,noun: complex sentence");
		}
		input.response_length = 2*1024;
	}
	if (args.fn == 3) {
		{
			auto& list = input.AddSub().Title("List \"B\" Classes of Sentences");
			list.NumberedLines();
			list.Add("noun phrase + independent clause");
			list.Add("independent clause + dependent clause");
			list.Add("prepositional phrase + independent clause");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("List \"B\" Categorizations of sentence structures");
			answer.NumberedLines();
			answer.Add("noun phrase + independent clause: declarative sentence");
			answer.Add("independent clause + dependent clause: conditional sentence");
			answer.Add("prepositional phrase + independent clause: descriptive sentence");
		}
		input.response_length = 2*1024;
	}
}

void AiTask::CreateInput_GetSourceDataAnalysis() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}

	SourceDataAnalysisArgs args;
	args.Put(this->args[0]);
	
	
	if (args.fn == 0) {
		Vector<String> lines = Split(args.text, "\n");
		ASSERT(lines.GetCount());
		{
			auto& txt = input.AddSub();
			txt.NoColon();
			txt.NoListChar();
			for (String& l : lines)
				txt.Add(l);
		}
		{
			auto& list = input.AddSub().Title("List A: terms to define a section");
			list.Add("exposition");
			list.Add("climax");
			list.Add("call to action");
			list.Add("high stakes obstacle");
			list.Add("rock bottom");
			list.Add("rising action");
			list.Add("falling action");
			list.Add("conclusion");
			list.Add("happy ending");
			list.Add("tragedy");
			list.Add("bittersweet ending");
			list.Add("suspense");
			list.Add("crisis");
			list.Add("resolution");
			list.Add("intensity");
			list.Add("conflict");
			list.Add("iteration");
			list.Add("etc.");
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("Analyze story and write matching word for all depths of sections (e.g. 0, 1.0, 2.1.0) from the list A (e.g. \"exposition\")");
			tmp_str = lines[0] + ": \"";
			results.Add(tmp_str);
		}
		input.response_length = 1024;
	}
	
	else if (args.fn == 1) {
		{
			auto& list = input.AddSub().Title("Artist: " + args.artist);
			list.NoColon();
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("Music genres");
			results.Add("");
		}
		input.response_length = 1024;
	}
	
	else if (args.fn == 4) {
		{
			auto& list = input.AddSub().Title("List of words \"A\"");
			list.Add("structure");
			list.Add("differently");
			list.Add("analyser");
		}
		{
			auto& list = input.AddSub().Title("Syllables and phonetic syllables of words \"A\"");
			list.Add("structure: struc-ture [strʌk.t͡ʃər]");
			list.Add("differently: dif-fer-ent-ly [ˈdɪ.fər.ənt.li]");
			list.Add("analyser: a-nal-y-ser [ˈæn.əl.əz.ər]");
		}
		{
			auto& list = input.AddSub().Title("List of words \"B\"");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("Syllables and phonetic syllables of words \"B\"");
			results.EmptyLine();
		}
		input.response_length = 2048;
	}

	else if (args.fn == 5) {
		{
			auto& list = input.AddSub().Title("Wordlist \"A\"");
			list.Add("introducing");
			list.Add("shameless");
		}
		{
			auto& list = input.AddSub().Title("Main class, metaphorical color in RGB value and Finnish translation for the wordlist \"A\"");
			list.Add("introducing: verb, RGB(0, 150, 255)");
			list.Add("shameless: adjective, RGB(255, 51, 153)");
		}
		{
			auto& list = input.AddSub().Title("Wordlist \"B\"");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("Main class, metaphorical color in RGB value for the wordlist \"B\"");
			results.EmptyLine();
		}
		input.response_length = 2048;
	}

	else if (args.fn == 7) {
		{
			auto& list = input.AddSub().Title("Word classes");
			list.Add("verb");
			list.Add("noun");
			list.Add("pronoun");
			list.Add("pronoun/noun");
			list.Add("preposition");
			list.Add("adjective");
			list.Add("modal verb");
			list.Add("adverb");
			list.Add("interjection");
			list.Add("conjunction");
			list.Add("contraction");
			list.Add("etc.");
		}
		{
			auto& list = input.AddSub().Title("Metaphoric color groups for words");
			for(int i = 0; i < GetColorGroupCount(); i++) {
				Color clr = GetGroupColor(i);
				String s;
				s << "RGB(" << (int)clr.GetR() << "," << (int)clr.GetG() << "," << (int)clr.GetB() << ")";
				list.Add(s);
			}
		}
		String pc = IntStr(1 + args.words.GetCount());
		{
			auto& list = input.AddSub().Title(pc + " words");
			//list.NumberedLines();
			list.Add("girly, adjective, RGB(255,192,203)");
			for(int i = 0; i < args.words.GetCount(); i++)
				list.Add(args.words[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			//results.NumberedLines();
			results.NoListChar();
			results.Title("100 words with 1-" + IntStr(GetColorGroupCount()) + " their similar alternatives with another metaphorical RGB color value (from the color group or a random color)");
			results.Add("adjective: girly -> RGB(255,140,0): sassy");
			results.Add("adjective: girly -> RGB(173,216,230): delicate");
			results.Add("adjective: girly -> RGB(160,82,45): bohemian");
			results.Add("adjective: girly -> RGB(0,0,128): romantic");
			results.Add("adjective: girly -> RGB(128,0,0): sexy");
			results.Add("adjective: girly -> RGB(255,0,0): bold");
			/*results.Add("adjective: girly -> RGB(255,0,0): bold");
			results.Add("adjective: girly -> RGB(128,0,0): sexy");
			results.Add("adjective: girly -> RGB(128,0,128): edgy");
			results.Add("adjective: girly -> RGB(0,0,128): romantic");
			results.Add("adjective: girly -> RGB(0,128,0): flirty");*/

			String s = args.words[0];
			s = s.Left(s.Find(","));
			results.Add(s + " ->");
		}
		input.response_length = 2048;
	}

	if (args.fn == 10 || args.fn == 11) {
		{
			auto& list = input.AddSub().Title("List \"A\": Word classes");
			list.Add("verb");
			list.Add("noun");
			list.Add("pronoun");
			list.Add("pronoun/noun");
			list.Add("preposition");
			list.Add("adjective");
			list.Add("modal verb");
			list.Add("adverb");
			list.Add("interjection");
			list.Add("conjunction");
			list.Add("contraction");
			list.Add("etc.");
		}
		{
			auto& list = input.AddSub().Title("List \"B\": Action planner action states for narrator person");
			list.Add("saying");
			list.Add("tone");
			list.Add("msg");
			list.Add("bias");
			list.Add("emotion");
			list.Add("level-of-certainty");
			list.Add("gesturing");
			list.Add("pointing");
			list.Add("describing-surroundings");
			list.Add("interrupting");
			list.Add("emphasizing");
			list.Add("summarizing");
			list.Add("referencing");
			list.Add("introducing");
			list.Add("concluding");
			list.Add("predicting");
			list.Add("transitioning");
			list.Add("questioning");
			list.Add("reflecting");
			list.Add("persuading");
			list.Add("comparing");
			list.Add("linking");
			list.Add("agreeing");
			list.Add("disagreeing");
			list.Add("apologizing");
			list.Add("commanding");
			list.Add("comforting");
			list.Add("complimenting");
			list.Add("complaining");
			list.Add("congratulating");
			list.Add("correcting");
			list.Add("denying");
			list.Add("explaining");
			list.Add("greeting");
			list.Add("inviting");
			list.Add("promising");
			list.Add("-suggesting");
			list.Add("thanking");
			list.Add("warning");
			list.Add("attention-attribute");
			list.Add("attention-person");
			list.Add("attention-person-implied");
			list.Add("attention-action");
			list.Add("attention-event");
			list.Add("attention-recipient");
			list.Add("attention-recipient-implied");
			list.Add("attention-relationship");
			list.Add("attention-purpose");
			list.Add("attention-place");
			list.Add("attention-time");
			list.Add("attention-topic");
			list.Add("attention-audience");
			list.Add("attention-occasion");
			list.Add("attention-conversation ");
			list.Add("attention-activity");
			list.Add("attention-emotional_state");
			list.Add("attention-physical_state");
			list.Add("attention-mental_state");
			list.Add("attention-relationship_status");
			list.Add("attention-goals");
			list.Add("attention-fears");
			list.Add("attention-preferences");
			list.Add("attention-beliefs");
			list.Add("attention-values");
			list.Add("attention-traits");
			list.Add("attention-education");
			list.Add("attention-work");
			list.Add("attention-hobbies");
			list.Add("attention-interests");
			list.Add("attention-achievement");
			list.Add("attention-experiences");
			list.Add("attention-likes");
			list.Add("attention-dislikes");
			list.Add("attention-tests");
			list.Add("attention-evaluation_criteria");
			list.Add("attention-qualifications");
			list.Add("attention-requirements");
			list.Add("attention-qualifications_acquired");
			list.Add("attention-qualifications_needed");
			list.Add("attention-suggestions");
			list.Add("attention-feedback");
			list.Add("attention-likes_dislikes_comments");
			list.Add("attention-expectations");
			list.Add("attention-motivations");
			list.Add("attention-priorities");
			list.Add("attention-challenges");
			list.Add("attention-opportunities");
			list.Add("attention-problems");
			list.Add("attention-decisions");
			list.Add("attention-recommendations");
			list.Add("attention-trial_discussion");
			list.Add("attention-agreement");
			list.Add("attention-disagreement");
			list.Add("attention-agreement-explanation");
			list.Add("attention-disagreement-explanation");
			list.Add("attention-reasoning");
			list.Add("attention-possibility");
			list.Add("attention-probability");
			list.Add("attention-improbable");
			list.Add("attention-necessity");
			list.Add("attention-priority");
			list.Add("attention-order");
			list.Add("attention-procedure");
			list.Add("attention-target");
			list.Add("attention-advocacy");
			list.Add("attention-advocacy-reasoning");
			list.Add("attention-evidences");
			list.Add("attention-negations");
			list.Add("attention-conclusions");
			list.Add("attention-persuasion");
			list.Add("attention-epiphany");
			list.Add("attention-choosing");
			list.Add("attention-concepts");
			list.Add("attention-situations");
			list.Add("attention-actionplan");
			list.Add("attention-outcome");
			list.Add("attention-plan-communication");
			list.Add("attention-plan-task");
			list.Add("attention-awakening");
			list.Add("attention-thinking");
			list.Add("attention-believing");
			list.Add("attention-knowing");
			list.Add("attention-learning");
			list.Add("attention-realization");
			list.Add("attention-incidences");
			list.Add("attention-causations");
			list.Add("attention-effects");
			list.Add("attention-solutions");
			list.Add("attention-progress");
			list.Add("attention-failure");
			list.Add("attention-change");
			list.Add("attention-impact");
			list.Add("attention-feeling");
			list.Add("attention-challenge");
			list.Add("attention-aspiration");
			list.Add("attention-doubt");
			list.Add("attention-relationship_goals");
			list.Add("attention-career_goals");
			list.Add("attention-emotional_goals");
			list.Add("attention-physical_goals");
			list.Add("attention-mental_goals");
			list.Add("attention-achievements");
			list.Add("attention-experiences_difficulties");
			list.Add("attention-explaining");
			list.Add("attention-analogy");
			list.Add("attention-fact");
			list.Add("attention-evidence");
			list.Add("attention-opinion");
			list.Add("attention-assumption");
			list.Add("attention-consequence");
			list.Add("attention-belief");
			list.Add("attention-value");
			list.Add("attention-confirmation");
			list.Add("attention-excuse");
			list.Add("attention-exception");
			list.Add("attention-exciting_feature");
			list.Add("attention-changemaker");
			list.Add("attention-mentor");
			list.Add("attention-friend");
			list.Add("attention-criticalopinion");
			list.Add("attention-conflict");
			list.Add("attention-perspective");
			list.Add("attention-prediction");
			list.Add("attention-regret");
			list.Add("attention-usefulness");
			list.Add("attention-solidarity");
			list.Add("attention-compliance");
			list.Add("attention-lack");
			list.Add("attention-attention");
			list.Add("attention-criticism");
			list.Add("attention-support");
			list.Add("attention-collaboration");
			list.Add("attention-anticipation");
			list.Add("attention-example");
			list.Add("etc.");
		}

	}

	if (args.fn == 10) {
		String pc = IntStr(3 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title(pc + " lines of lyrics");
			list.NumberedLines();
			list.Add("2 AM, howlin outside");
			list.Add("Lookin, but I cannot find");
			list.Add("Only you can stand my mind");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title("Action planner action states for " + pc + " lines of lyrics. With the most matching actions of list \"B\"");
			results.Add("tone(urgent) + msg(trying to reach someone) + bias(romantic) + emotion(uncertainty) + level-of-certainty(trying/desire) + gesturing(pointing) + describing-surroundings(anywhere in the dark) + attention-place(outside) + attention-time(night) + attention-emotional_state(desire) + attention-action(howling) + attention-activity(driving)");
			results.Add("msg(searching for someone) + bias(doubt) + emotion(frustration) + level-of-certainty(cannot find) + attention-action(searching) + attention-relationship(checking for person's presence)");
			results.Add("tone(affectionate) + msg(expressing feelings) + bias(feeling understood by person) + emotion(love) + level-of-certainty(statement) + attention-person(addressed to person) + attention-emotional_state(love/affection) + attention-mental_state(thinking about person constantly) + attention-relationship(checking for compatibility)");
			results.Add("");
		}
		input.response_length = 2048;
	}

	if (args.fn == 11) {
		String audience = "audience";
		{
			auto& list = input.AddSub().Title("Action planner heuristic score factors");
			list.Add("S0: High like count from the " + audience + ". Low count means that the idea behind the phrase was bad.");
			list.Add("S1: High comment count from the " + audience + ". Low count means that there was no emotion in the phrase.");
			list.Add("S2: High listen count from the " + audience + ". Low count means that there was bad so called hook in the phrase.");
			list.Add("S3: High share count from the " + audience + ". Low count means that the phrase was not relatable.");
			list.Add("S4: High bookmark count from the " + audience + ". Low count means that the phrase had no value.");
			list.Add("S5: High reference count towards comedy from the " + audience + ". Low count means that the phrase was not funny.");
			list.Add("S6: High reference count towards sex from the " + audience + ". Low count means that the phrase was not sensual.");
			list.Add("S7: High reference count towards politics from the " + audience + ". Low count means that the phrase was not thought-provoking.");
			list.Add("S8: High reference count towards love from the " + audience + ". Low count means that the phrase was not romantic.");
			list.Add("S9: High reference count towards social issues from the " + audience + ". Low count means that the phrase was not impactful.");
		}
		{
			auto& list = input.AddSub().Title("Change of actions between 2 lines. Score of stopping actions in the first line and value of starting actions in the second line. Scores and score factors. Value is between 0-10");
			list.Add("Stop line 1 & start line 2: S0: 0, S1: 0, S2: 7, S3: 3, S4: 0, S5: 2, S6: 3, S7: 5, S8: 7, S9: 1");
			list.Add("Stop line 2 & start line 3: S0: 2, S1: 0, S2: 2, S3: 1, S4: 0, S5: 4, S6: 3, S7: 2, S8: 9, S9: 6");
			list.Add("Stop line 3 & start line 4: S0: 1, S1: 5, S2: 3, S3: 2, S4: 8, S5: 8, S6: 6, S7: 9, S8: 4, S9: 2");
		}
		String pc = IntStr(0 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title("List \"C\": Actions per " + pc + " lines of lyrics. With the most matching actions of list \"B\"");
			list.NumberedLines();
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title("Change of actions between 2 lines in list \"C\" with " + pc + " lines of actions. Score of stopping actions in the first line and value of starting actions in the second line. Scores and score factors S0-S9. Value is between 0-10:");
			results.Add("Stop line 1 & start line 2: S0:");
		}
		input.response_length = 1024;
	}

}

void AiTask::CreateInput_GetPhraseData() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	
	PhraseArgs args;
	args.Put(this->args[0]);
	
	enum {
		PHASE_ELEMENT,
		PHASE_COLOR,
		PHASE_ATTR,
		PHASE_ACTIONS,
		PHASE_SCORES,
		PHASE_TYPECLASS,
		PHASE_CONTENT,
		
		PHASE_COUNT
	};
	
	if (args.fn == PHASE_COLOR) {
		{
			auto& list = input.AddSub().Title("List \"A\" of sentences");
			list.NumberedLines();
			list.Add("everyone of us loves her");
			list.Add("they need to be silenced");
			list.Add("you need help and we can contribute");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("Metaphorical RGB colors of sentences of list \"A\"");
			answer.NumberedLines();
			answer.Add("RGB(153, 255, 153)");
			answer.Add("RGB(153, 0, 0)");
			answer.Add("RGB(255, 153, 204)");
		}
		input.response_length = 2*1024;
	}
	else if (args.fn == PHASE_ATTR) {
		{
			TaskTitledList& list = input.AddSub().Title("List of attribute groups and their opposite polarised attribute values");
			list.NumberedLines();
			#define ATTR_ITEM(e, g, i0, i1) list.Add(g ": " i0); list.Add(g ": " i1);
			ATTR_LIST
			#undef ATTR_ITEM
		}
		{
			auto& list = input.AddSub().Title("List \"A\" of sentences");
			list.NumberedLines();
			list.Add("everyone of us loves her");
			list.Add("they need to be silenced");
			list.Add("you need help and we can contribute");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			auto& answer = input.PreAnswer();
			answer.Title("Matching group and value for sentences of list \"A\"");
			answer.NumberedLines();
			answer.Add("belief communities: acceptance");
			answer.Add("theological opposites: authoritarian");
			answer.Add("faith and reason seekers: rational thinker");
		}
		input.response_length = 1024*3/2;
	}
	else if (args.fn == PHASE_ACTIONS) {
		{
			// NOTE duplicate
			auto& list = input.AddSub().Title("List \"B\": Action planner action states for narrator person");
			list.Add("saying");
			list.Add("tone");
			list.Add("msg");
			list.Add("bias");
			list.Add("emotion");
			list.Add("level-of-certainty");
			list.Add("gesturing");
			list.Add("pointing");
			list.Add("describing-surroundings");
			list.Add("interrupting");
			list.Add("emphasizing");
			list.Add("summarizing");
			list.Add("referencing");
			list.Add("introducing");
			list.Add("concluding");
			list.Add("predicting");
			list.Add("transitioning");
			list.Add("questioning");
			list.Add("reflecting");
			list.Add("persuading");
			list.Add("comparing");
			list.Add("linking");
			list.Add("agreeing");
			list.Add("disagreeing");
			list.Add("apologizing");
			list.Add("commanding");
			list.Add("comforting");
			list.Add("complimenting");
			list.Add("complaining");
			list.Add("congratulating");
			list.Add("correcting");
			list.Add("denying");
			list.Add("explaining");
			list.Add("greeting");
			list.Add("inviting");
			list.Add("promising");
			list.Add("-suggesting");
			list.Add("thanking");
			list.Add("warning");
			list.Add("attention-attribute");
			list.Add("attention-person");
			list.Add("attention-person-implied");
			list.Add("attention-action");
			list.Add("attention-event");
			list.Add("attention-recipient");
			list.Add("attention-recipient-implied");
			list.Add("attention-relationship");
			list.Add("attention-purpose");
			list.Add("attention-place");
			list.Add("attention-time");
			list.Add("attention-topic");
			list.Add("attention-audience");
			list.Add("attention-occasion");
			list.Add("attention-conversation ");
			list.Add("attention-activity");
			list.Add("attention-emotional_state");
			list.Add("attention-physical_state");
			list.Add("attention-mental_state");
			list.Add("attention-relationship_status");
			list.Add("attention-goals");
			list.Add("attention-fears");
			list.Add("attention-preferences");
			list.Add("attention-beliefs");
			list.Add("attention-values");
			list.Add("attention-traits");
			list.Add("attention-education");
			list.Add("attention-work");
			list.Add("attention-hobbies");
			list.Add("attention-interests");
			list.Add("attention-achievement");
			list.Add("attention-experiences");
			list.Add("attention-likes");
			list.Add("attention-dislikes");
			list.Add("attention-tests");
			list.Add("attention-evaluation_criteria");
			list.Add("attention-qualifications");
			list.Add("attention-requirements");
			list.Add("attention-qualifications_acquired");
			list.Add("attention-qualifications_needed");
			list.Add("attention-suggestions");
			list.Add("attention-feedback");
			list.Add("attention-likes_dislikes_comments");
			list.Add("attention-expectations");
			list.Add("attention-motivations");
			list.Add("attention-priorities");
			list.Add("attention-challenges");
			list.Add("attention-opportunities");
			list.Add("attention-problems");
			list.Add("attention-decisions");
			list.Add("attention-recommendations");
			list.Add("attention-trial_discussion");
			list.Add("attention-agreement");
			list.Add("attention-disagreement");
			list.Add("attention-agreement-explanation");
			list.Add("attention-disagreement-explanation");
			list.Add("attention-reasoning");
			list.Add("attention-possibility");
			list.Add("attention-probability");
			list.Add("attention-improbable");
			list.Add("attention-necessity");
			list.Add("attention-priority");
			list.Add("attention-order");
			list.Add("attention-procedure");
			list.Add("attention-target");
			list.Add("attention-advocacy");
			list.Add("attention-advocacy-reasoning");
			list.Add("attention-evidences");
			list.Add("attention-negations");
			list.Add("attention-conclusions");
			list.Add("attention-persuasion");
			list.Add("attention-epiphany");
			list.Add("attention-choosing");
			list.Add("attention-concepts");
			list.Add("attention-situations");
			list.Add("attention-actionplan");
			list.Add("attention-outcome");
			list.Add("attention-plan-communication");
			list.Add("attention-plan-task");
			list.Add("attention-awakening");
			list.Add("attention-thinking");
			list.Add("attention-believing");
			list.Add("attention-knowing");
			list.Add("attention-learning");
			list.Add("attention-realization");
			list.Add("attention-incidences");
			list.Add("attention-causations");
			list.Add("attention-effects");
			list.Add("attention-solutions");
			list.Add("attention-progress");
			list.Add("attention-failure");
			list.Add("attention-change");
			list.Add("attention-impact");
			list.Add("attention-feeling");
			list.Add("attention-challenge");
			list.Add("attention-aspiration");
			list.Add("attention-doubt");
			list.Add("attention-relationship_goals");
			list.Add("attention-career_goals");
			list.Add("attention-emotional_goals");
			list.Add("attention-physical_goals");
			list.Add("attention-mental_goals");
			list.Add("attention-achievements");
			list.Add("attention-experiences_difficulties");
			list.Add("attention-explaining");
			list.Add("attention-analogy");
			list.Add("attention-fact");
			list.Add("attention-evidence");
			list.Add("attention-opinion");
			list.Add("attention-assumption");
			list.Add("attention-consequence");
			list.Add("attention-belief");
			list.Add("attention-value");
			list.Add("attention-confirmation");
			list.Add("attention-excuse");
			list.Add("attention-exception");
			list.Add("attention-exciting_feature");
			list.Add("attention-changemaker");
			list.Add("attention-mentor");
			list.Add("attention-friend");
			list.Add("attention-criticalopinion");
			list.Add("attention-conflict");
			list.Add("attention-perspective");
			list.Add("attention-prediction");
			list.Add("attention-regret");
			list.Add("attention-usefulness");
			list.Add("attention-solidarity");
			list.Add("attention-compliance");
			list.Add("attention-lack");
			list.Add("attention-attention");
			list.Add("attention-criticism");
			list.Add("attention-support");
			list.Add("attention-collaboration");
			list.Add("attention-anticipation");
			list.Add("attention-example");
			list.Add("etc.");
		}
		
		String pc = IntStr(3 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title(pc + " lines of lyrics");
			list.NumberedLines();
			list.Add("2 AM, howlin outside");
			list.Add("Lookin, but I cannot find");
			list.Add("Only you can stand my mind");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title("Action planner action states for " + pc + " lines of lyrics. With the most matching actions of list \"B\"");
			results.Add("tone(urgent) + msg(trying to reach someone) + bias(romantic) + emotion(uncertainty) + level-of-certainty(trying/desire) + gesturing(pointing) + describing-surroundings(anywhere in the dark) + attention-place(outside) + attention-time(night) + attention-emotional_state(desire) + attention-action(howling) + attention-activity(driving)");
			results.Add("msg(searching for someone) + bias(doubt) + emotion(frustration) + level-of-certainty(cannot find) + attention-action(searching) + attention-relationship(checking for person's presence)");
			results.Add("tone(affectionate) + msg(expressing feelings) + bias(feeling understood by person) + emotion(love) + level-of-certainty(statement) + attention-person(addressed to person) + attention-emotional_state(love/affection) + attention-mental_state(thinking about person constantly) + attention-relationship(checking for compatibility)");
			results.Add("");
		}
		input.response_length = 2048;
	}
	else if (args.fn == PHASE_SCORES) {
		String audience = "audience";
		{
			auto& list = input.AddSub().Title("Action planner heuristic score factors");
			list.Add("S0: High like count from the " + audience + ". Low count means that the idea behind the phrase was bad.");
			list.Add("S1: High comment count from the " + audience + ". Low count means that there was no emotion in the phrase.");
			list.Add("S2: High listen count from the " + audience + ". Low count means that there was bad so called hook in the phrase.");
			list.Add("S3: High share count from the " + audience + ". Low count means that the phrase was not relatable.");
			list.Add("S4: High bookmark count from the " + audience + ". Low count means that the phrase had no value.");
			list.Add("S5: High reference count towards comedy from the " + audience + ". Low count means that the phrase was not funny.");
			list.Add("S6: High reference count towards sex from the " + audience + ". Low count means that the phrase was not sensual.");
			list.Add("S7: High reference count towards politics from the " + audience + ". Low count means that the phrase was not thought-provoking.");
			list.Add("S8: High reference count towards love from the " + audience + ". Low count means that the phrase was not romantic.");
			list.Add("S9: High reference count towards social issues from the " + audience + ". Low count means that the phrase was not impactful.");
		}
		{
			auto& list = input.AddSub().Title("Example 1");
			list.Add("Score factors are S0-S9");
			list.Add("The value of a score factor is between 0-10");
			list.Add("Phrase is \"bleeding after you\"");
			list.Add("Score factors for the phrase \"bleeding after you\": S0: 9, S1: 8, S2: 8, S3: 6, S4: 7, S5: 9, S6: 4, S7: 2, S8: 3, S9: 2");
			list.Add("The score factors in shortened format: 9 8 8 6 7 9 4 2 3 2");
		}
		
		String pc = IntStr(1 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" of " + pc + " phrases, with example arguments for making the action plan");
			list.NumberedLines();
			list.Add("bleeding after you");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title(pc + " score factors for list \"A\" of phrases");
			results.Add("9 8 8 6 7 9 4 2 3 2");
		}
		input.response_length = 2048;
	}
	else if (args.fn == PHASE_TYPECLASS) {
		{
			auto& list = input.AddSub().Title("Typeclasses of artist profiles in relation to the lyrics");
			list.NumberedLines();
			ASSERT(args.typeclasses.GetCount());
			for (String tc : args.typeclasses)
				list.Add(tc);
		}
		String pc = IntStr(1 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" of " + pc + " phrases");
			list.NumberedLines();
			list.Add("bleeding after you");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title(pc + " typeclass-number sequences for list \"A\" of phrases. Description: phrases can be used in these (numbered) typeclasses");
			results.Add("1 51 42 10 11 13 24 28 30 44");
			results.Add("");
		}
		input.response_length = 2048;
	}
	else if (args.fn == PHASE_CONTENT) {
		{
			auto& list = input.AddSub().Title("List of names for archetypical parts of storyline of a modern pop/rock/edm songs, which contrasts each other");
			list.NumberedLines();
			ASSERT(args.contents.GetCount());
			for (const auto& it : args.contents)
				list.Add(it);
		}
		String pc = IntStr(1 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" of " + pc + " phrases");
			list.NumberedLines();
			list.Add("bleeding after you");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title(pc + " archetypical part number-alpha sequences for list \"A\" of phrases. Description: phrases would fit to following storyline parts");
			results.Add("33A 15C 9B 31B 32A 34B 36C 27C 40C");
			results.Add("");
		}
		input.response_length = 1024*3/2;
	}
	else if (args.fn == PHASE_ELEMENT) {
		{
			auto& list = input.AddSub().Title("List of conceptual elements of storyline of a modern pop/rock/edm songs");
			list.NumberedLines();
			for (const auto& e : args.elements) {
				list.Add(e);
			}
		}
		String pc = IntStr(1 + args.phrases.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" of " + pc + " phrases");
			list.NumberedLines();
			list.Add("bleeding after you");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.NumberedLines();
			results.NoListChar();
			results.Title(pc + " conceptual elements for list \"A\" of phrases. Description: some element of the list that would be closest fit to following storyline parts");
			results.Add("exposition");
			results.Add("");
			tmp_str = "2. ";
		}
		input.response_length = 1024*3/2;
	}
}

void AiTask::CreateInput_GetActionAnalysis() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	
	ActionAnalysisArgs args;
	args.Put(this->args[0]);
	
	
	if (args.fn == 0) {
		{
			auto& list = input.AddSub().Title("Lyrics");
			list.NoListChar();
			list.Add("2 AM, howlin outside");
			list.Add("Lookin, but I cannot find");
		}
		
		{
			auto& list = input.AddSub().Title("Actions per a line of lyrics. With the most matching actions of list \"B\"");
			list.NoListChar();
			list.Add("\"2 AM, howlin outside\": attention-time(night) + attention-emotional_state(desire) + attention-action(howling) + attention-activity(driving) + tone(urgent) + msg(trying to reach someone) + bias(romantic + emotion(uncertainty) + level-of-certainty(trying/desire) + gesturing(pointing) + describing-surroundings(anywhere in the dark) + attention-place(outside)");
			list.Add("\"Lookin, but I cannot find\": attention-action(looking) + attention-physical state(tired) + emotion(frustration) + attention-emotional_state(desperation) + attention-time(late at night)");
		}
		
		{
			auto& list = input.AddSub().Title("Phrases with the metaphorical color RGB integer (r,g,b) code at the end of the line");
			list.NoListChar();
			list.Add("\"sassy\": RGB(255,140,0)");
			list.Add("\"golden opportunities\": RGB(255,215,0)");
			list.Add("\"blue ocean, green trees, live in harmony\", RGB(0,128,0)");
			list.Add("\"2 AM, howlin outside\", RGB(0,0,128)");
			list.Add("\"Lookin, but I cannot find\", RGB(128,128,128)");
		}
		
		{
			auto& list = input.AddSub().Title("Actions of the list \"C\"");
			list.NoListChar();
			list.Add("\"attention-event(unpleasant smell)\"");
			list.Add("\"msg(expressing physical desire)\"");
			for (const String& s : args.actions)
				list.Add("\"" + s + "\"");
		}
		
		{
			auto& answer = input.PreAnswer();
			answer.Title("Metaphorical RGB value of actions in the list \"C\"");
			answer.NoListChar();
			answer.Add("\"attention-event(unpleasant smell)\" RGB(128,0,0)");
			answer.Add("\"msg(expressing physical desire)\" RGB(255, 192, 203)");
			answer.Add("");
		}
		input.response_length = 2*1024;
	}
	if (args.fn == 1) {
		{
			auto& list = input.AddSub().Title("Lyrics");
			list.NoListChar();
			list.Add("2 AM, howlin outside");
			list.Add("Lookin, but I cannot find");
		}
		
		{
			auto& list = input.AddSub().Title("Actions per a line of lyrics. With the most matching actions of list \"B\"");
			list.NoListChar();
			list.Add("\"2 AM, howlin outside\": attention-time(night) + attention-emotional_state(desire) + attention-action(howling) + attention-activity(driving) + tone(urgent) + msg(trying to reach someone) + bias(romantic + emotion(uncertainty) + level-of-certainty(trying/desire) + gesturing(pointing) + describing-surroundings(anywhere in the dark) + attention-place(outside)");
			list.Add("\"Lookin, but I cannot find\": attention-action(looking) + attention-physical state(tired) + emotion(frustration) + attention-emotional_state(desperation) + attention-time(late at night)");
		}
		
		{
			auto& list = input.AddSub().Title("Attribute list \"A\" (key, group, primary value, opposite value)");
			#define ATTR_ITEM(e, g, i0, i1) list.Add(g " / " i0 " / " i1);
			ATTR_LIST
			#undef ATTR_ITEM
		}
		
		{
			auto& list = input.AddSub().Title("Primary attribute groups and values of sentences from attribute list \"A\"");
			list.Add("\"I won't blindly follow the crowd\": group faith / individual spirituality");
			list.Add("\"feeling blue and green with envy\": sexual preference / kinky");
			list.Add("\"2 AM, howlin outside\": faith and reason seekers / divine worshipers");
			list.Add("\"Lookin, but I cannot find\": truthfulness / personal experience");
		}
		
		{
			auto& list = input.AddSub().Title("Actions of the list \"C\"");
			list.NoListChar();
			list.Add("\"attention-event(unpleasant smell)\"");
			list.Add("\"transition(activities/roles)\"");
			for (const String& s : args.actions)
				list.Add("\"" + s + "\"");
		}
		
		{
			auto& answer = input.PreAnswer();
			answer.Title("Primary attribute groups and values of sentences from attribute list \"A\" for actions of the list \"C\"");
			answer.NoListChar();
			answer.Add("\"attention-event(unpleasant smell)\" sexualization / non-sexual");
			answer.Add("\"transition(activities/roles)\" integrity / twisted");
			answer.Add("");
		}
		input.response_length = 2*1024;
	}
}

void AiTask::CreateInput_GetAttributes() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	
	AttrArgs args;
	args.Put(this->args[0]);
	
	if (args.fn == 0) {
		auto& list = input.AddSub().Title("List \"A\" values in the same group '" + args.group + "'");
		int end = min(200, args.values.GetCount());
		for(int i = 0; i < end; i++) {
			const String& s = args.values[i];
			list.Add(s);
		}
		if (end > 2) {
			TaskTitledList& results = input.PreAnswer();
			results.Title("2 main values of list \"A\", which summarizes all values in a way, that the first value is the common attribute of modern pop/rock/edm songs, and the second value is the polar opposite of the first");
			results.NumberedLines();
			results.Add("");
		}
		else {
			list.NumberedLines();
			TaskTitledList& results = input.PreAnswer();
			results.Title("Sort 2 values of list \"A\" in a way, that the first value is the once which is closer to a common attribute of modern pop/rock/edm songs. Use same values, but just sort the values. Don't add any text");
			results.NumberedLines();
			results.Add("");
			tmp_str = "1. ";
		}
		input.response_length = 2048;
	}
	else if (args.fn == 1) {
		ASSERT(args.attr0.GetCount());
		ASSERT(args.attr1.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" values in the same group 'socioeconomic status'");
			list.NumberedLines();
			list.Add("urban");
			list.Add("gang affiliation");
			list.Add("drug dealing");
		}
		{
			auto& list = input.AddSub().Title("List \"B\" polar opposites of the group 'socioeconomic status'");
			list.Add("positive: wealth");
			list.Add("negative: poverty");
		}
		{
			auto& list = input.AddSub().Title("Values of list \"A\", with their closest polar opposite value of list \"B\". Either 'positive' or 'negative");
			list.NumberedLines();
			list.Add("positive");
			list.Add("negative");
			list.Add("negative");
		}
		{
			auto& list = input.AddSub().Title("List \"C\" values in the same group '" + args.group + "'");
			list.NumberedLines();
			for(int i = 0; i < args.values.GetCount(); i++) {
				const String& s = args.values[i];
				list.Add(s);
			}
		}
		{
			auto& list = input.AddSub().Title("List \"D\" polar opposites of the group '" + args.group + "'");
			list.Add("positive: " + args.attr0);
			list.Add("negative: " + args.attr1);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("Values of list \"C\", with their closest polar opposite value of list \"D\". Either 'positive' or 'negative");
			results.NumberedLines();
			results.Add("");
			//results.Add(args.values[0] + ":");
			//tmp_str = args.values[0] + ":";
		}
		input.response_length = 2048;
	}
	else if (args.fn == 2) {
		ASSERT(args.groups.GetCount());
		ASSERT(args.values.GetCount());
		{
			auto& list = input.AddSub().Title("List \"A\" attribute groups with polarised extremes");
			list.NumberedLines();
			for(int i = 0; i < args.groups.GetCount(); i++)
				list.Add(args.groups[i]);
		}
		{
			auto& list = input.AddSub().Title("List \"B\" orphaned groups/value pairs");
			list.NumberedLines();
			list.Add("culture: mainstream success");
			for(int i = 0; i < args.values.GetCount(); i++)
				list.Add(args.values[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title("For the values of the list \"B\", their closest matching group and polarised extreme value from the list \"A\"");
			results.NumberedLines();
			results.Add("5 +");
			results.Add("");
			//results.Add(args.values[0] + ":");
			//tmp_str = args.values[0] + ":";
		}
		input.response_length = 2048;
	}
}

void AiTask::CreateInput_ScriptSolver() {
	if (args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	
	ScriptSolverArgs args;
	args.Put(this->args[0]);
	
	if (args.fn == 18) {
		String first_word, second_word;
		{
			auto& list = input.AddSub().Title("Lyrics A");
			list.NoListChar();
			for(int i = 0; i < args.phrases.GetCount(); i++) {
				String& s = args.phrases[i];
				if (i == 0 && first_word.IsEmpty()) {
					WString ws = s.ToWString();
					int a = ws.Find(" ");
					if (a <= 1)
						a = ws.Find(" ", a+1);
					if (a >= 0)
						first_word = ws.Left(a).ToString();
				}
				if (i == 1 && second_word.IsEmpty()) {
					WString ws = s.ToWString();
					int a = ws.Find(" ");
					if (a <= 1)
						a = ws.Find(" ", a+1);
					if (a >= 0)
						second_word = ws.Left(a).ToString();
				}
				list.Add(s);
			}
		}
		{
			auto& list = input.AddSub().Title("Lyrics B");
			list.NoListChar();
			for (String& s : args.phrases2)
				list.Add(s);
		}
		if (args.elements.GetCount()) {
			auto& list = input.AddSub().Title("New lyrics should fit the following elements");
			for (String& l : args.elements)
				list.Add(l);
		}
		#if 0
		{
			auto& list = input.AddSub().Title("New lyrics should use same pronouns than Lyrics A");
			list.NoColon();
		}
		if (0) {
			auto& list = input.AddSub().Title("New lyrics should use same allegories than Lyrics A, if there's any allegories in Lyrics A (there might not be)");
			list.NoColon();
		}
		{
			auto& list = input.AddSub().Title("New lyrics should have the same number of syllables than Lyrics A");
			list.NoColon();
		}
		{
			auto& list = input.AddSub().Title("New lyrics should have the same level of hopelessness, bitterness, cynicism and negativity than Lyrics A");
			list.NoColon();
		}
		{
			auto& list = input.AddSub().Title("New lyrics should have the same focus on negativity and negative outcome than lyrics A");
			list.NoColon();
		}
		#endif
		{
			TaskTitledList& results = input.PreAnswer();
			//String t = "Create a list of new lyrics that combines elements of two existing lyrics to convey a style of A but message of B"
			//	". This lyrics should be " + IntStr(args.phrases.GetCount()) + " line inline & end rhyme and in slight dialect";
			String t = "Create a list of new lyrics that combines elements of two existing lyrics. New lyrics should morph lyrics A to have the context of lyrics B"
				". This lyrics should be " + IntStr(args.phrases.GetCount()) + " line inline & end rhyme";
			if (first_word.GetCount()) {
				t += ". All lines should begin with '" + first_word + "'";
				if (second_word.GetCount())
					t += " and have ''" + second_word + "' in the second line";
			}
			results.Title(t);
			results.NumberedLines();
			tmp_str = first_word + " ";
			results.Add(first_word);
		}
		input.response_length = 2048;
	}
	else if (args.fn == 19) {
		if (args.previously.GetCount()){
			if (args.previously.GetCount() > 10000)
				args.previously = args.previously.Left(10000) + "...(too long)";
			auto& list = input.AddSub().Title("Happened previously in the story so far");
			list.NoListChar();
			for (const auto& str : Split(args.previously, "\n"))
				list.Add(str);
		}
		
		for(int i = -1; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title =
				i == -1 ?
					String("Properties for all lines (unless specified)") :
					Format("Line #%d", i+1);
			auto& list = input.AddSub().Title(title);
			if (state.element.GetCount()) list.Add("conceptual element", state.element);
			if (state.attr_group.GetCount()) list.Add("attribute-group", state.attr_group);
			if (state.attr_value.GetCount()) list.Add("attribute-value", state.attr_value);
			if (state.clr_i >= 0) {
				Color c = GetGroupColor(state.clr_i);
				list.Add("metaphorical color", Format("RGB(%d,%d,%d)", (int)c.GetR(), (int)c.GetG(), (int)c.GetB()));
			}
			if (state.act_action.GetCount()) list.Add("action", state.act_action + "(" + args.state.act_arg + ")");
			else if (state.act_arg.GetCount()) list.Add("action-argument", args.state.act_arg);
			if (state.typeclass.GetCount()) list.Add("typeclass", state.typeclass);
			if (state.content.GetCount()) list.Add("content", state.content);
			if (state.content_mod.GetCount()) list.Add("content-mod", state.content_mod);
		}
		
		String first_word, second_word;
		{
			auto& list = input.AddSub().Title("Lyrics");
			list.NoListChar();
			for(int i = 0; i < args.phrases.GetCount(); i++) {
				String& s = args.phrases[i];
				if (i == 0 && first_word.IsEmpty()) {
					WString ws = s.ToWString();
					int a = ws.Find(" ");
					if (a <= 1)
						a = ws.Find(" ", a+1);
					if (a >= 0)
						first_word = ws.Left(a).ToString();
				}
				if (i == 1 && second_word.IsEmpty()) {
					WString ws = s.ToWString();
					int a = ws.Find(" ");
					if (a <= 1)
						a = ws.Find(" ", a+1);
					if (a >= 0)
						second_word = ws.Left(a).ToString();
				}
				list.Add(s);
			}
		}
		if (args.peek.GetCount()){
			auto& list = input.AddSub().Title("Peeking the upcoming lyrics");
			list.NoListChar();
			for (const auto& str : Split(args.peek, "\n"))
				list.Add(str);
		}
		
		{
			TaskTitledList& results = input.PreAnswer();
			String t = "Write a part of a story, which matches lyrics";
			if (args.previously.GetCount())
				t += " and continues the previously happened story";
			if (args.peek.GetCount())
				t += " and prepares for the upcoming lyrics (but don't write about them yet)";
			t +=". The story should be for adults and it is allowed to contain NSFW, dirty, sexually explicit, unsafe and explicit language. The story should be written in dialect";
			results.Title(t);
			results.NoListChar();
			results.Add("");
		}
		input.response_length = 1024;
	}
	else if (args.fn == 20) {
		int limit = 9000 / (1 + args.line_states.GetCount());
		
		if (args.previously.GetCount()){
			if (args.previously.GetCount() > limit)
				args.previously = args.previously.Left(limit) + "...(too long)";
			auto& list = input.AddSub().Title("Happened previously in the story so far");
			list.NoListChar();
			for (const auto& str : Split(args.previously, "\n"))
				list.Add(str);
		}
		
		for(int i = 0; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title = Format("Sub-part #%d", i+1);
			auto& list = input.AddSub().Title(title);
			if (state.element.GetCount()) list.Add("conceptual element", state.element);
			if (state.attr_group.GetCount()) list.Add("attribute-group", state.attr_group);
			if (state.attr_value.GetCount()) list.Add("attribute-value", state.attr_value);
			if (state.clr_i >= 0) {
				Color c = GetGroupColor(state.clr_i);
				list.Add("metaphorical color", Format("RGB(%d,%d,%d)", (int)c.GetR(), (int)c.GetG(), (int)c.GetB()));
			}
			if (state.act_action.GetCount()) list.Add("action", state.act_action + "(" + args.state.act_arg + ")");
			else if (state.act_arg.GetCount()) list.Add("action-argument", args.state.act_arg);
			if (state.typeclass.GetCount()) list.Add("typeclass", state.typeclass);
			if (state.content.GetCount()) list.Add("content", state.content);
			if (state.content_mod.GetCount()) list.Add("content-mod", state.content_mod);
			String s = args.phrases[i];
			if (s.GetCount() > limit)
				s = s.Left(limit) + "...(too long story to continue)";
			list.Add("story (multiline)", s);
		}
		
		if (args.peek.GetCount()){
			auto& list = input.AddSub().Title("Peeking the upcoming lyrics");
			list.NoListChar();
			for (const auto& str : Split(args.peek, "\n"))
				list.Add(str);
		}
		
		{
			TaskTitledList& results = input.PreAnswer();
			String t = "Combine sub-parts of the story to a single short part of the story";
			if (args.previously.GetCount())
				t += ", which continues the previously happened story";
			if (args.peek.GetCount())
				t += ", which prepares for the upcoming lyrics (but don't write about them yet)";
			t +=". The story should be for adults and it is allowed to contain NSFW, dirty, sexually explicit, unsafe and explicit language. The story should be written in dialect";
			results.Title(t);
			results.NoListChar();
			results.Add("");
		}
		input.response_length = 1024;
	}
	else if (args.fn == 21) {
		const char* conn_str[3] = {
			"And then (2 events happen consecutively)",
			"Therefore (an event happens because something happened)",
			"But (something happens, but then something surprising happens)",
		};
		{
			auto& list = input.AddSub().Title("3 types of contextual connectors (which word connects the 2 line together the best)");
			for(int i = 0; i < 3; i++)
				list.Add(conn_str[i]);
		}
		{
			auto& list = input.AddSub().Title("Context of the lyrics");
			list.NumberedLines();
			int limit = 4000;
			int per_phrase_limit = limit / args.phrases2.GetCount();
			for(int i = 0; i < args.phrases2.GetCount(); i++) {
				String& s = args.phrases2[i];
				s.Replace("\n", " ");
				if (s.GetCount() > per_phrase_limit)
					s = s.Left(per_phrase_limit);
				list.Add(s);
			}
		}
		{
			auto& list = input.AddSub().Title("Draft of lyrics");
			for(int i = 0; i < args.phrases.GetCount(); i++)
				list.Add(args.phrases[i]);
		}
		for(int i = 0; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title = Format("Information about the line #%d", i+1);
			auto& list = input.AddSub().Title(title);
			if (state.content.GetCount()) list.Add("text", state.content);
			if (state.style_type.GetCount()) list.Add("style type", state.style_type);
			if (state.style_entity.GetCount()) list.Add("mimic artist", state.style_entity);
			list.Add("safety", state.safety ? "unsafe (cursing, sexual acts, dirty words, genital words, contempt, etc. are allowed)" : "safe");
			if (state.connector >= 0 && state.connector < 3)
				list.Add("the line should connect to the previous line with", conn_str[state.connector]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			String t = "Expand \"" + args.ref + "\" using the given context to be more understable";
			results.Title(t);
			results.NoListChar();
			results.Add("");
		}
		input.response_length = 1024;
	}
	else if (args.fn == 22) {
		const char* len_str[4] = {
			"Long (10 words)",
			"Medium (7 word)",
			"Short (4 words)",
			"Very short (2 words)"
		};
		const char* conn_str[3] = {
			"And then (2 events happen consecutively)",
			"Therefore (an event happens because something happened)",
			"But (something happens, but then something surprising happens)",
		};
		{
			auto& list = input.AddSub().Title("Example of a 2 line lyrics");
			list.Add("\"On a Friday night, we be wildin' out / Everybody wants something, no doubt\"");
		}
		{
			auto& list = input.AddSub().Title("3 types of contextual connectors (which word connects the 2 line together the best)");
			for(int i = 0; i < 3; i++)
				list.Add(conn_str[i]);
		}
		{
			auto& list = input.AddSub().Title("4 types of lengths of lines");
			for(int i = 0; i < 4; i++)
				list.Add(len_str[i]);
		}
		for(int i = 0; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title = Format("Line #%d", i+1);
			auto& list = input.AddSub().Title(title);
			list.Add("draft text", args.phrases[i]);
			list.Add("expanded draft text", args.phrases2[i]);
			if (state.element.GetCount()) list.Add("conceptual element", state.element);
			if (state.attr_group.GetCount()) list.Add("attribute-group", state.attr_group);
			if (state.attr_value.GetCount()) list.Add("attribute-value", state.attr_value);
			if (state.clr_i >= 0) {
				Color c = GetGroupColor(state.clr_i);
				list.Add("metaphorical color", Format("RGB(%d,%d,%d)", (int)c.GetR(), (int)c.GetG(), (int)c.GetB()));
			}
			if (state.act_action.GetCount()) list.Add("action", state.act_action + "(" + args.state.act_arg + ")");
			else if (state.act_arg.GetCount()) list.Add("action-argument", args.state.act_arg);
			if (state.typeclass.GetCount()) list.Add("typeclass", state.typeclass);
			if (state.content.GetCount()) list.Add("content", state.content);
			if (state.content_mod.GetCount()) list.Add("content-mod", state.content_mod);
		}
		for(int i = 0; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title = Format("Writing instructions for the new line #%d", i+1);
			auto& list = input.AddSub().Title(title);
			if (state.style_type.GetCount()) list.Add("style type", state.style_type);
			if (state.style_entity.GetCount()) list.Add("mimic artist", state.style_entity);
			list.Add("safety", state.safety ? "unsafe (cursing, sexual acts, dirty words, genital words, contempt, etc. are allowed)" : "safe");
			if (state.line_len >= 0 && state.line_len < 4)
				list.Add("the length should be", len_str[state.line_len]);
			if (state.connector >= 0 && state.connector < 3)
				list.Add("the line should connect to the previous line with", conn_str[state.connector]);
			if (state.line_begin.GetCount()) list.Add("the line should begin with the word", state.line_begin);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			String t = "Create a list of different propositions that combines elements of two existing lines #1 and #2.  Propositions are not consecutive sentences"
				". This lyrics should be " + IntStr(args.phrases.GetCount()) + " line inline & end rhyme";
			//t += ". The style should match the style of Nicki Minaj";
			t += ". The lyrics should be explicit and in dialect";
			results.Title(t);
			results.NumberedLines();
			tmp_str = "\"";
			results.Add(tmp_str);
		}
		input.response_length = 2048;
	}
	else if (args.fn == 23) {
		for(int i = 0; i < args.line_states.GetCount(); i++) {
			auto& state = i == -1 ? args.state : args.line_states[i];
			String title = Format("Line #%d", i+1);
			auto& list = input.AddSub().Title(title);
			list.Add("draft text", args.phrases[i]);
			list.Add("expanded draft text", args.phrases2[i]);
			if (state.element.GetCount()) list.Add("conceptual element", state.element);
			if (state.attr_group.GetCount()) list.Add("attribute-group", state.attr_group);
			if (state.attr_value.GetCount()) list.Add("attribute-value", state.attr_value);
			if (state.clr_i >= 0) {
				Color c = GetGroupColor(state.clr_i);
				list.Add("metaphorical color", Format("RGB(%d,%d,%d)", (int)c.GetR(), (int)c.GetG(), (int)c.GetB()));
			}
			if (state.act_action.GetCount()) list.Add("action", state.act_action + "(" + args.state.act_arg + ")");
			else if (state.act_arg.GetCount()) list.Add("action-argument", args.state.act_arg);
			if (state.typeclass.GetCount()) list.Add("typeclass", state.typeclass);
			if (state.content.GetCount()) list.Add("content", state.content);
			if (state.content_mod.GetCount()) list.Add("content-mod", state.content_mod);
		}
		{
			String title = ("Possible styles");
			auto& list = input.AddSub().Title(title);
			for(int i = 0; i < args.styles.GetCount(); i++) {
				list.Add("#" + IntStr(i) + ": " + args.styles[i]);
			}
		}
		{
			TaskTitledList& results = input.PreAnswer();
			String t = "Get top 3 styles for given lines";
			results.Title(t);
			results.NumberedLines();
			results.Add("#");
		}
		input.response_length = 2048;
	}
	else TODO
}

END_UPP_NAMESPACE

