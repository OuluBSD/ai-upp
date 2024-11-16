#include "AI.h"

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

END_UPP_NAMESPACE

