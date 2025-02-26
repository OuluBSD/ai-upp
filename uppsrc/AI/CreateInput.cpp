#include "AI.h"
#include <AI/Core/Defs.h>

NAMESPACE_UPP

void AiTask::CreateInput_GetTokenData() {}
void AiTask::CreateInput_GetPhraseData() {}
void AiTask::CreateInput_GetAttributes() {}

void AiTask::CreateInput_Translate()
{
	String orig_lng = args[0];
	String orig_txt = args[1];
	String trans_lng = args[2];
	bool slightly_dialect = StrInt(args[3]);

	Vector<String> lines = Split(orig_txt, "\n", false);

	TaskTitledList& in_orig = input.AddSub().Title("Text 1 in " + orig_lng);
	in_orig.NoListChar();
	for(const String& line : lines)
		in_orig.Add(line);

	String t = "Text 1 in " + trans_lng;
	if(slightly_dialect)
		t += " and in slightly dialect";

	TaskTitledList& results = input.PreAnswer();
	results.Title(t);

	input.response_length = 1024 * 2;
}

void AiTask::CreateInput_CreateImage()
{
	int count = StrInt(args[1]);
	int reduce_size_mode = StrInt(args[2]);
	int size = 0;
	switch(reduce_size_mode) {
	case 0:
		size = 1024;
		break;
	case 1:
		size = 512;
		break;
	case 2:
		size = 256;
		break;
	default:
		SetError("invalid 'reduce_size_mode'");
		return;
	}
	image_sz = IntStr(size) + "x" + IntStr(size);
	image_n = IntStr(count);

	input.PreAnswer().NoColon().Title(args[0]);
}

void AiTask::CreateInput_EditImage()
{
	int count = StrInt(args[1]);
	Image orig = send_images[0];
	int size = 0;
	Size sz = orig.GetSize();
	if(sz.cx != sz.cy) {
		SetError("Image must be square");
		return;
	}
	switch(sz.cx) {
	case 1024:
		size = 1024;
		break;
	case 512:
		size = 512;
		break;
	case 256:
		size = 256;
		break;
	default:
		SetError("invalid 'size'");
		return;
	}
	image_sz = IntStr(size) + "x" + IntStr(size);
	image_n = IntStr(count);

	input.PreAnswer().NoColon().Title(args[0]);

	skip_load = true;
}

void AiTask::CreateInput_VariateImage()
{
	int count = StrInt(args[0]);
	Image orig = send_images[0];
	int size = 0;
	Size sz = orig.GetSize();
	if(sz.cx != sz.cy) {
		SetFatalError("Image must be square");
		return;
	}
	switch(sz.cx) {
	case 1024:
		size = 1024;
		break;
	case 512:
		size = 512;
		break;
	case 256:
		size = 256;
		break;
	default:
		SetFatalError("invalid 'size'");
		return;
	}
	image_sz = IntStr(size) + "x" + IntStr(size);
	image_n = IntStr(count);

	input.PreAnswer().NoColon().Title(
		"DUMMY PROMPT! NEVER SENT IN VARIATE MODE! PREVENTS FAILING FOR 'NO-INPUT'");

	// skip_load = true;
}

void AiTask::CreateInput_Vision()
{
	this->type = TYPE_VISION;

	if(args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}

	VisionArgs args;
	args.Put(this->args[0]);

	if(args.fn == 0) {
		{
			input.AddSub()
				.Title("Task: describe content of the image in a detailed way, which enables "
			           "the regeneration of the image using generative AI")
				.NoColon();
		}
	}
	else
		SetError("Invalid function");
}

void AiTask::CreateInput_Transcription()
{
	this->type = TYPE_TRANSCRIPTION;
	raw_input = this->args[0]; // hash is made of raw_input == arguments
}

void AiTask::CreateInput_Default()
{
	if(args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}

	TaskArgs args;
	args.Put(this->args[0]);

	if(args.fn == FN_VOICEOVER_1_FIND_NATURAL_PARTS) {
		ValueMap params = args.params;
		ValueArray lines = params("lines");
		{
			auto& list = input.AddSub()
				.Title("Transcript")
				.NoListChar();
			for(int i = 0; i < lines.GetCount(); i++)
				list.Add("line #" + IntStr(i) + " " + lines[i].ToString());
		}{
			auto& results = input.PreAnswer()
				.Title("List of 1-4 best lines  to split transcript");
			results.Add("line #");
		}
		input.response_length = 50;
		tmp_str = "line #";
	}
	else if(args.fn == FN_VOICEOVER_2A_SUMMARIZE) {
		ValueMap params = args.params;
		String scene = params("scene");
		String people = params("people");
		String language = params("language");
		if (language.IsEmpty()) language = "English";
		String transcript = params("transcription_part");
		input.AddSub().Title("Scene").NoListChar().Add(scene);
		input.AddSub().Title("People").NoListChar().Add(people);
		if (params.Find("total_summarization") >= 0)
			input.AddSub().Title("Summarization of all parts").NoListChar().Add(params("total_summarization"));
		if (params.Find("prev_summarization") >= 0)
			input.AddSub().Title("Summarization of previous part").NoListChar().Add(params("prev_summarization"));
		input.AddSub().Title("Transcript of the current part").NoListChar().Add(transcript);
		{
			auto& results = input.PreAnswer()
				.Title("Make voiceover summarization of transcription of the current part in " + language + " (from 1st person perspective)")
				.NoListChar();
			results.Add("");
		}
		input.response_length = 2048;
	}
	else if(args.fn == FN_VOICEOVER_2B_SUMMARIZE_TOTAL) {
		ValueMap params = args.params;
		String language = params("language");
		if (language.IsEmpty()) language = "English";
		input.AddSub().Title("Summarization of all previous parts").NoListChar().Add(params("total_summarization"));
		input.AddSub().Title("Summarization of current part").NoListChar().Add(params("summarization"));
		{
			auto& results = input.PreAnswer()
				.Title("Merge summarization of all previous parts and summarization of current part in " + language)
				.NoListChar();
			results.Add("");
		}
		input.response_length = 2048;
	}
	else if(args.fn == FN_TRANSCRIPT_PROOFREAD_1) {
		String text = args.params("text");
		TranscriptResponse r;
		LoadFromJson(r, text);
		String misspelled = args.params("misspelled");
		if (!misspelled.IsEmpty()) {
			auto& l = input.AddSub()
				.Title("Misspelled words");
			misspelled.Replace(",", " ");
			Vector<String> words = Split(misspelled, " ");
			for (auto& w : words)
				l.Add(w);
		}{
			auto& l = input.AddSub()
				.Title("List of dialog segments in " + r.language)
				.NoListChar();
			for(int i = 0; i < r.segments.GetCount(); i++) {
				l.Add("#" + IntStr(i) + ": " + r.segments[i].text);
			}
		}{
			auto& results = input.PreAnswer()
				.Title("List of proofread of previous text in English, and with unique lines only, and with corrected grammar. Line format (- #original line: translation). Skip original lines with duplicate text or low content value")
				;
			results.Add("#0:");
		}
		input.response_length = 2048;
	}
	else if(args.fn == FN_PROOFREAD_STORYLINE_1 ||
			args.fn == FN_STORYLINE_DIALOG_1) {
		ValueArray arr = args.params("proofread");
		String scene = args.params("scene");
		String people = args.params("people");
		String storyline = args.params("storyline");
		{
			auto& l = input.AddSub()
				.Title("List of dialog segments")
				.NumberedLines();
			for(int i = 0; i < arr.GetCount(); i++) {
				l.Add(arr[i]);
			}
		}
		if (!scene.IsEmpty()) {
			auto& l = input.AddSub()
				.Title("Scene")
				.NoListChar();
			l.Add(scene);
		}
		if (!people.IsEmpty()) {
			auto& l = input.AddSub()
				.Title("People")
				.NoListChar();
			l.Add(people);
		}
		if (!storyline.IsEmpty()) {
			auto& l = input.AddSub()
				.Title("Storyline")
				.NoListChar();
			l.Add(storyline);
		}
		if (args.fn == FN_PROOFREAD_STORYLINE_1) {
			auto& results = input.PreAnswer()
				.Title("Storyline of previous list")
				.NoListChar();
			results.Add("");
		}
		else if (args.fn == FN_STORYLINE_DIALOG_1) {
			auto& results = input.PreAnswer()
				.Title("New dialog for a play based on the original dialog and the storyline")
				.NoListChar();
			results.Add("");
		}
		input.response_length = 2048;
	}
	else if(args.fn == FN_STORYLINE_DIALOG_1) {
		
		
		
	}
	else if(args.fn == FN_ANALYZE_ELEMENTS) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
		    "query": {
		        "__comment__": {
		            "incomplete list of elements to define a part of a script": [
		                "exposition",
		                "climax",
		                "call to action",
		                "high stakes obstacle",
		                "rock bottom",
		                "rising action",
		                "falling action",
		                "conclusion",
		                "happy ending",
		                "tragedy",
		                "bittersweet ending",
		                "suspense",
		                "crisis",
		                "resolution",
		                "intensity",
		                "conflict",
		                "iteration"
		            ]
		        },
		        "script": {
		            "text": [
		                [
		                    "Hello"
		                ]
		            ],
		            "__comment__": "Analyze script and write matching word for all depths of script from the list A (e.g. \"exposition\")"
		        }
		    },
		    "response-full": {
		        "text & elements": [
		            [
		                [
		                    "Hello",
		                    "greeting"
		                ]
		            ]
		        ]
		    },
		    "response-short": {
		        "elements": [
		            [
		                "greeting"
		            ]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "text": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/text", args.params("text"));
		input.response_length = 2048;
	}
	else if(args.fn == FN_WORD_CLASSES) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
			"query": {
				"__comment__": {
					"incomplete list of word classes": [
						"Nouns",
						"Verbs",
						"Adjectives",
						"Adverbs",
						"Pronouns",
						"Prepositions",
						"Conjunctions",
						"Determiners",
						"Interjections",
						"Articles",
						"Modal verbs",
						"Gerunds",
						"Infinitives",
						"Participles",
						"Definite article",
						"Indefinite article",
						"Proper nouns",
						"Collective nouns",
						"Concrete nouns",
						"Abstract nouns",
						"Irregular verbs",
						"Regular verbs",
						"Transitive verbs",
						"Intransitive verbs",
						"Auxiliary verbs",
						"Reflexive verbs",
						"Imperative verbs",
						"First person pronouns",
						"Second person pronouns",
						"Third person pronouns",
						"Possessive pronouns",
						"Demonstrative pronouns",
						"Relative pronouns",
						"Intensive pronouns",
						"Indefinite pronouns",
						"Personal pronouns",
						"Subject pronouns",
						"Objective pronouns",
						"Possessive determiners",
						"Possessive adjectives",
						"Comparative adjectives",
						"Superlative adjectives",
						"Proper adjectives",
						"Positive adjectives",
						"Negative adjectives",
						"etc."
					]
				},
				"script": {
					"words": [
						[
							"you",
							"what's",
							"smile"
						]
					],
					"__comment__": "Analyze words and write word classes"
				}
			},
			"response-full": {
				"words & word classes": [
					[
						[
							"you",
							"pronoun"
						],
						[
							"what's",
							"contraction (what + is)"
						],
						[
							"smile",
							"noun | verb"
						]
					]
				]
			},
			"response-short": {
				"word classes": [
					[
						"pronoun",
						"contraction (what + is)",
						"noun | verb"
					]
				]
			}
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "words": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/words", args.params("words"));
		input.response_length = 2048;
	}
	else if (args.fn == FN_WORD_PAIR_CLASSES) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML({
		    "query": {
		        "__comment__": {
		            "incomplete list of word classes": [
		                "Nouns",
		                "Verbs",
		                "Adjectives",
		                "Adverbs",
		                "Pronouns",
		                "Prepositions",
		                "Conjunctions",
		                "Determiners",
		                "Interjections",
		                "Articles",
		                "Modal verbs",
		                "Gerunds",
		                "Infinitives",
		                "Participles",
		                "Definite article",
		                "Indefinite article",
		                "Proper nouns",
		                "Collective nouns",
		                "Concrete nouns",
		                "Abstract nouns",
		                "Irregular verbs",
		                "Regular verbs",
		                "Transitive verbs",
		                "Intransitive verbs",
		                "Auxiliary verbs",
		                "Reflexive verbs",
		                "Imperative verbs",
		                "First person pronouns",
		                "Second person pronouns",
		                "Third person pronouns",
		                "Possessive pronouns",
		                "Demonstrative pronouns",
		                "Relative pronouns",
		                "Intensive pronouns",
		                "Indefinite pronouns",
		                "Personal pronouns",
		                "Subject pronouns",
		                "Objective pronouns",
		                "Possessive determiners",
		                "Possessive adjectives",
		                "Comparative adjectives",
		                "Superlative adjectives",
		                "Proper adjectives",
		                "Positive adjectives",
		                "Negative adjectives"
		            ]
		        },
		        "script": {
		            "word pairs": [
		                [
		                    "automobile",
		                    "drives"
		                ]
		            ],
		            "__comment__": "Analyze word pairs and write word classes"
		        }
		    },
		    "response-full": {
		        "word pairs & word pairs classes": [
		            [
		                [
		                    "automobile",
		                    "drives"
		                ],
		                [
		                    "noun",
		                    "verb"
		                ]
		            ]
		        ]
		    },
		    "response-short": {
		        "word pairs classes": [
		            [
		                [
		                    "noun",
		                    "verb"
		                ]
		            ]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "word pairs": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/word pairs", args.params("word pairs"));
		input.response_length = 2*1024;
	}
	else if (args.fn == FN_CLASSIFY_SENTENCE) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML({
			"query": {
				"__comment__": {
					"incomplete list of sentence structures": [
						"declarative sentence",
						"conditional sentence",
						"descriptive sentence",
						"causal sentence",
						"subject-verb-object sentence",
						"subject-verb-adjective sentence",
						"etc."
					],
					"incomplete list of classes of sentences": [
						"independent clause",
						"dependent clause",
						"coordinating clause",
						"modifying clause",
						"non-coordinating clause",
						"subordinating clause",
						"etc."
					]
				},
				"script": {
					"classified_sentences": [
						[
							["noun","verb","adjective"],
							["adjective","noun","preposition","noun"],
							["conjunction","pronoun","verb","noun"]
						]
					],
					"__comment__": "Analyze classified sentences based on word classes"
				}
			},
			"response-full": {
				"titles_of_classified_sentences": [
					[
						[["noun","verb","adjective"], "independent clause"],
						[["adjective","noun","preposition","noun"], "prepositional sentence"],
						[["conjunction","pronoun","verb","noun"], "complex sentence"]
					]
				]
			},
			"response-short": {
				"titles": [
					[
						"independent clause",
						"prepositional sentence",
						"complex sentence"
					]
				]
			}
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "classified_sentences": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/classified_sentences", args.params("classified_sentences"));
		input.response_length = 2*1024;
	}
	else if (args.fn == FN_CLASSIFY_SENTENCE_STRUCTURES) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML({
		    "query": {
		        "script": {
		            "classes of sentences": [
		                [
		                    "noun phrase",
		                    "independent clause"
		                ],
		                [
		                    "independent clause",
		                    "dependent clause"
		                ],
		                [
		                    "prepositional phrase",
		                    "independent clause"
		                ]
		            ],
		            "__comment__": "Identify sentence structures"
		        }
		    },
		    "response-full": {
		        "Sentence structure categorizations": [
		            [
		                [
		                    "noun phrase",
		                    "independent clause"
		                ],
		                "declarative sentence"
		            ],
		            [
		                [
		                    "independent clause",
		                    "dependent clause"
		                ],
		                "conditional sentence"
		            ],
		            [
		                [
		                    "prepositional phrase",
		                    "independent clause"
		                ],
		                "descriptive sentence"
		            ]
		        ]
		    },
		    "response-short": {
		        "categories": [
		            "declarative sentence",
		            "conditional sentence",
		            "descriptive sentence"
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "classes of sentences": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/classified_sentences", args.params("classified_sentences"));
		input.response_length = 2*1024;
	}
	
	else if (args.fn == FN_CLASSIFY_PHRASE_COLOR) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML({
		    "query": {
		        "script": {
		            "sentences": [
						"everyone of us loves her",
						"they need to be silenced",
						"you need help and we can contribute"
					],
		            "__comment__": "Identify sentence metaphorical colors"
				}
		    },
		    "response-full": {
		         "__comment__": "Syntax translation: RGB(128,255,0) <-> [128,255,0]",
		        "sentences & metaphorical RGB colors": [
					["everyone of us loves her", [153, 255, 153]],
					["they need to be silenced",[153, 0, 0]],
					["you need help and we can contribute", [255, 153, 204]]
				]
			},
		    "response-short": {
		        "metaphorical RGB colors": [
					[153, 255, 153],
					[153, 0, 0],
					[255, 153, 204]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "classes of sentences": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/sentences", args.params("sentences"));
		input.response_length = 2*1024;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_ATTR) {
		json_input.AddDefaultSystem();
		json_input.AddAssist((String)R"ML({
		    "query": {
		        "script": {
		            "__comment__": {
						"info": "List of attribute groups and their opposite polarised attribute values",
						"format": ["group", "positive extreme", "negative extreme"],
						"list": [)ML"
						#define ATTR_ITEM(a,b,c,d) "[\"" b "\",\"" c "\",\"" d "\"],"
						ATTR_LIST
						#undef ATTR_ITEM
						"[]"
						R"ML(]
					},
		            "sentences": [
						"everyone of us loves her",
						"they need to be silenced",
						"you need help and we can contribute"
					]
				}
			},
		    "response-full": {
				"group_and_value_matches": [
					{
						"text": "everyone of us loves her",
						"attribute": "belief communities",
						"value": "acceptance"
					},
					{
						"text": "they need to be silenced",
						"attribute": "theological opposites",
						"value": "authoritarian"
					},
					{
						"text": "you need help and we can contribute",
						"attribute": "faith and reason seekers",
						"value": "rational thinker"
					}
				]
			},
			"response-short": {
				"group_and_value_matches": [
					["belief communities", "acceptance"],
					["theological opposites", "authoritarian"],
					["attribute", "rational thinker"]
				]
			}
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "sentences": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/sentences", args.params("sentences"));
		input.response_length = 2*1024;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_ACTIONS) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML({
		  "query": {
		    "__comment__": {
		      "incomplete list of action planner's action states for a narrating person": [
		        "saying",
		        "tone",
		        "msg",
		        "bias",
		        "emotion",
		        "level-of-certainty",
		        "gesturing",
		        "pointing",
		        "describing-surroundings",
		        "interrupting",
		        "emphasizing",
		        "summarizing",
		        "referencing",
		        "introducing",
		        "concluding",
		        "predicting",
		        "transitioning",
		        "questioning",
		        "reflecting",
		        "persuading",
		        "comparing",
		        "linking",
		        "agreeing",
		        "disagreeing",
		        "apologizing",
		        "commanding",
		        "comforting",
		        "complimenting",
		        "complaining",
		        "congratulating",
		        "correcting",
		        "denying",
		        "explaining",
		        "greeting",
		        "inviting",
		        "promising",
		        "suggesting",
		        "thanking",
		        "warning",
		        "attention-attribute",
		        "attention-person",
		        "attention-person-implied",
		        "attention-action",
		        "attention-event",
		        "attention-recipient",
		        "attention-recipient-implied",
		        "attention-relationship",
		        "attention-purpose",
		        "attention-place",
		        "attention-time",
		        "attention-topic",
		        "attention-audience",
		        "attention-occasion",
		        "attention-conversation ",
		        "attention-activity",
		        "attention-emotional_state",
		        "attention-physical_state",
		        "attention-mental_state",
		        "attention-relationship_status",
		        "attention-goals",
		        "attention-fears",
		        "attention-preferences",
		        "attention-beliefs",
		        "attention-values",
		        "attention-traits",
		        "attention-education",
		        "attention-work",
		        "attention-hobbies",
		        "attention-interests",
		        "attention-achievement",
		        "attention-experiences",
		        "attention-likes",
		        "attention-dislikes",
		        "attention-tests",
		        "attention-evaluation_criteria",
		        "attention-qualifications",
		        "attention-requirements",
		        "attention-qualifications_acquired",
		        "attention-qualifications_needed",
		        "attention-suggestions",
		        "attention-feedback",
		        "attention-likes_dislikes_comments",
		        "attention-expectations",
		        "attention-motivations",
		        "attention-priorities",
		        "attention-challenges",
		        "attention-opportunities",
		        "attention-problems",
		        "attention-decisions",
		        "attention-recommendations",
		        "attention-trial_discussion",
		        "attention-agreement",
		        "attention-disagreement",
		        "attention-agreement-explanation",
		        "attention-disagreement-explanation",
		        "attention-reasoning",
		        "attention-possibility",
		        "attention-probability",
		        "attention-improbable",
		        "attention-necessity",
		        "attention-priority",
		        "attention-order",
		        "attention-procedure",
		        "attention-target",
		        "attention-advocacy",
		        "attention-advocacy-reasoning",
		        "attention-evidences",
		        "attention-negations",
		        "attention-conclusions",
		        "attention-persuasion",
		        "attention-epiphany",
		        "attention-choosing",
		        "attention-concepts",
		        "attention-situations",
		        "attention-actionplan",
		        "attention-outcome",
		        "attention-plan-communication",
		        "attention-plan-task",
		        "attention-awakening",
		        "attention-thinking",
		        "attention-believing",
		        "attention-knowing",
		        "attention-learning",
		        "attention-realization",
		        "attention-incidences",
		        "attention-causations",
		        "attention-effects",
		        "attention-solutions",
		        "attention-progress",
		        "attention-failure",
		        "attention-change",
		        "attention-impact",
		        "attention-feeling",
		        "attention-challenge",
		        "attention-aspiration",
		        "attention-doubt",
		        "attention-relationship_goals",
		        "attention-career_goals",
		        "attention-emotional_goals",
		        "attention-physical_goals",
		        "attention-mental_goals",
		        "attention-achievements",
		        "attention-experiences_difficulties",
		        "attention-explaining",
		        "attention-analogy",
		        "attention-fact",
		        "attention-evidence",
		        "attention-opinion",
		        "attention-assumption",
		        "attention-consequence",
		        "attention-belief",
		        "attention-value",
		        "attention-confirmation",
		        "attention-excuse",
		        "attention-exception",
		        "attention-exciting_feature",
		        "attention-changemaker",
		        "attention-mentor",
		        "attention-friend",
		        "attention-criticalopinion",
		        "attention-conflict",
		        "attention-perspective",
		        "attention-prediction",
		        "attention-regret",
		        "attention-usefulness",
		        "attention-solidarity",
		        "attention-compliance",
		        "attention-lack",
		        "attention-attention",
		        "attention-criticism",
		        "attention-support",
		        "attention-collaboration",
		        "attention-anticipation",
		        "attention-example"
		      ]
		    },
		    "script": {
		      "phrases": [
		        [
		          "2 AM, howlin outside",
		          "Lookin, but I cannot find",
		          "Only you can stand my mind"
		        ]
		      ],
		      "__comment__": "analyze action planner action states for phrases"
		    }
		  },
		  "response-full": {
		    "phrases & action states": [
		      {"text":"2 AM, howlin outside", "actions":[["tone","urgent"], ["msg","trying to reach someone"], ["bias","romantic"], ["emotion","uncertainty"], ["level-of-certainty","trying/desire"], ["gesturing","pointing"], ["describing-surroundings","anywhere in the dark"], ["attention-place","outside"], ["attention-time","night"], ["attention-emotional_state","desire"], ["attention-action","howling"], ["attention-activity","driving"]]},
		      {"text":"Lookin, but I cannot find", "actions":[["msg","searching for someone"], ["bias","doubt"], ["emotion","frustration"], ["level-of-certainty","cannot find"], ["attention-action","searching"], ["attention-relationship","checking for person's presence"]]},
		      {"text":"Only you can stand my mind", "actions":[["tone","affectionate"], ["msg","expressing feelings"], ["bias","feeling understood by person"], ["emotion","love"], ["level-of-certainty","statement"], ["attention-person","addressed to person"], ["attention-emotional_state","love/affection"], ["attention-mental_state","thinking about person constantly"], ["attention-relationship","checking for compatibility"]]}
		    ]
		  },
		  "response-short": {
		    "action states": [
		      [["tone","urgent"], ["msg","trying to reach someone"], ["bias","romantic"], ["emotion","uncertainty"], ["level-of-certainty","trying/desire"], ["gesturing","pointing"], ["describing-surroundings","anywhere in the dark"], ["attention-place","outside"], ["attention-time","night"], ["attention-emotional_state","desire"], ["attention-action","howling"], ["attention-activity","driving"]],
		      [["msg","searching for someone"], ["bias","doubt"], ["emotion","frustration"], ["level-of-certainty","cannot find"], ["attention-action","searching"], ["attention-relationship","checking for person's presence"]],
		      [["tone","affectionate"], ["msg","expressing feelings"], ["bias","feeling understood by person"], ["emotion","love"], ["level-of-certainty","statement"], ["attention-person","addressed to person"], ["attention-emotional_state","love/affection"], ["attention-mental_state","thinking about person constantly"], ["attention-relationship","checking for compatibility"]]
		    ]
		  }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "phrases": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/phrases", args.params("phrases"));
		input.response_length = 2*1024;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_SCORES) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
		    "query": {
		        "__comment__": {
					"heuristic_score_factors": {
						"S0": "High like count from the audience. Low count means that the idea behind the phrase was bad.",
						"S1": "High comment count from the audience. Low count means that there was no emotion in the phrase.",
						"S2": "High listen count from the audience. Low count means that there was bad so-called hook in the phrase.",
						"S3": "High share count from the audience. Low count means that the phrase was not relatable.",
						"S4": "High bookmark count from the audience. Low count means that the phrase had no value.",
						"S5": "High reference count towards comedy from the audience. Low count means that the phrase was not funny.",
						"S6": "High reference count towards sex from the audience. Low count means that the phrase was not sensual.",
						"S7": "High reference count towards politics from the audience. Low count means that the phrase was not thought-provoking.",
						"S8": "High reference count towards love from the audience. Low count means that the phrase was not romantic.",
						"S9": "High reference count towards social issues from the audience. Low count means that the phrase was not impactful."
					},
					"description": [
						"Score factors are S0-S9 and can be considered as a vector of 10 integer numbers",
						"The value of a score factor is between 0-10",
						"Phrase is \"bleeding after you\"",
						"Score factors for the phrase \"bleeding after you\": S0: 9, S1: 8, S2: 8, S3: 6, S4: 7, S5: 9, S6: 4, S7: 2, S8: 3, S9: 2",
						"The score factors in shortened format: 9 8 8 6 7 9 4 2 3 2"
					]
				},
		        "script": {
		            "phrases": [
						"bleeding after you"
		            ],
		            "__comment__": "Analyze phrases and give rating (values S0-S9)"
		        }
		    },
		    "response-full": {
		        "phrases & score factors": [
	                [
	                    "bleeding after you",
	                    [9, 8, 8, 6, 7, 9, 4, 2, 3, 2]
	                ]
		        ]
		    },
		    "response-short": {
		        "score factors": [
		            [9, 8, 8, 6, 7, 9, 4, 2, 3, 2]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "phrases": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/phrases", args.params("phrases"));
		input.response_length = 2048;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_TYPECLASS) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
			"query": {
				"__comment__": {
					"incomplete list of typeclasses of profiles in relation to the text": {
						0: "Heartbroken/lovesick",
						1: "Rebel/anti-establishment",
						2: "Political activist",
						3: "Social justice advocate",
						4: "Party/club",
						5: "Hopeful/dreamer",
						6: "Confident/empowered",
						7: "Vulnerable/raw",
						8: "Romantic/love-driven",
						9: "Failure/loser",
						10: "Spiritual/faithful",
						11: "Passionate/determined",
						12: "Reflective/self-reflective",
						13: "Witty/sarcastic",
						14: "Melancholic/sad",
						15: "Humble/down-to-earth",
						16: "Charismatic/charming",
						17: "Resilient/overcoming adversity",
						18: "Carefree/joyful",
						19: "Dark/mysterious",
						20: "Comical/humorous",
						21: "Controversial/provocative",
						22: "Nostalgic/sentimental",
						23: "Wise/philosophical",
						24: "Angry/outspoken",
						25: "Calm/peaceful.",
						26: "Confident/self-assured",
						27: "Self-destructive/self-sabotaging",
						28: "Hopeful/optimistic",
						29: "Fearful/anxious",
						30: "Eccentric/quirky",
						31: "Sensitive/emotional",
						32: "Bitter/resentful",
						33: "Unique/nonconformist",
						34: "Free-spirited/nonconformist",
						35: "Sultry/seductive",
						36: "Inspirational/motivational",
						37: "Authentic/real",
						38: "Mysterious/enigmatic",
						39: "Carefree/bohemian",
						40: "Street-smart/tough",
						41: "Romantic/idealistic",
						42: "Nurturing/motherly",
						43: "Dark/tormented",
						44: "Remorseful/regretful",
						45: "Bold/brave",
						46: "Outcast/rebel",
						47: "Lost/disconnected",
						48: "Tough/badass",
						49: "Sincere/genuine",
						50: "Honest/vulnerable",
						51: "Innocent/naive",
						52: "Bold/risk-taking"
					}
				},
		        "script": {
		            "phrases": [
						"bleeding after you",
					],
		            "__comment__": "Analyze phrases and give one or more typeclasses than could use this phrase"
				}
		    },
		    "response-full": {
		        "phrases & typeclasses": [
	                [
	                    "bleeding after you",
	                    [1, 51, 42, 10, 11, 13, 24, 28, 30, 44]
	                ]
		        ]
		    },
		    "response-short": {
		        "score factors": [
					[1, 51, 42, 10, 11, 13, 24, 28, 30, 44]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "phrases": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/phrases", args.params("phrases"));
		input.response_length = 2048;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_CONTENT) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
			"query": {
				"__comment__": {
					"incomplete list of short storylines": {
						0: "Seductive intro",
						1: "Rise and fall",
						2: "Fun and games",
						3: "Love at first sight",
						4: "Struggle and triumph",
						5: "Ups and downs",
						6: "Escape to paradise",
						7: "Rebellious spirit",
						8: "Broken and mended",
						9: "Chase your dreams",
						10: "Dark secrets",
						11: "Rags to riches",
						12: "Lost and found",
						13: "Ignite the fire",
						14: "From the ashes",
						15: "Fame and fortune",
						16: "Healing in the darkness",
						17: "City lights and lonely nights",
						18: "Breaking the mold",
						19: "Haunted by the past",
						20: "Wild and free",
						21: "Clash of opinions",
						22: "Long distance love",
						23: "Finding inner strength",
						24: "Living a double life",
						25: "Caught in the spotlight",
						26: "Love and war",
						27: "The art of letting go",
						28: "Living in the moment",
						29: "Conquering fears"
					},
					"incomplete list of 3 phases of a storyline": {
						"generic": ["beginning", "middle", "end"],
						"encoded": ["A", "B", "C"],
						"Rise and fall": ["rise, "neutral", "fall"],
						"Escape to paradise": ["not in paradise", "going to paradise", "in paradise"]
					}
				},
		        "script": {
		            "phrases": [
						"bleeding after you"
					],
		            "__comment__": "Analyze phrases and write the matching storyline and 1 one of the 3 phases"
				}
		    },
		    "response-full": {
				"__comment__": "the phase is always in the encoded format: e.g. beginning = A",
		        "phrases & storylines": [
	                [
	                    "bleeding after you",
	                    [[10, "A"], [2, "C"], [5, "B"], [6, "A"], [26,"B"], [27,"A"]]
	                ]
		        ]
		    },
		    "response-short": {
		        "storylines": [
					[[10, "A"], [2, "C"], [5, "B"], [6, "A"], [26,"B"], [27,"A"]]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "phrases": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/phrases", args.params("phrases"));
		input.response_length = 2048;
	}
	else if (args.fn == FN_CLASSIFY_ACTION_COLOR) {
		json_input.AddDefaultSystem();
		json_input.AddAssist(R"ML(
		{
			"query": {
				"__comment__": {
					"examples of phrases and actions (group & value)": [
						["2 AM, howlin outside", [
							["attention-time","night"],
							["attention-emotional_state","desire"],
							["attention-action","howling"],
							["attention-activity","driving"],
							["tone","urgent"],
							["msg","trying to reach someone"],
							["bias","romantic"],
							["emotion","uncertainty"],
							["level-of-certainty","trying/desire"],
							["gesturing","pointing"],
							["describing-surroundings","anywhere in the dark"],
							["attention-place","outside"]
						]],
						["Lookin, but I cannot find", [
							["attention-action","looking"],
							["attention-physical state","tired"],
							["emotion","frustration"],
							["attention-emotional_state","desperation"],
							["attention-time","late at night"]
						]]
					]
				},
		        "script": {
					"__comment1__": "actions have two parts: group & value",
		            "actions": [
						["attention-event","unpleasant smell"],
						["msg","expressing physical desire"]
					],
					"__comment2__": "Analyze actions and write matchin metaphorical colors"
				}
		    },
		    "response-full": {
				"__comment__": "Syntax translation: RGB(128,255,0) <-> [128,255,0]",
				"actions & colors": [
					[["attention-event","unpleasant smell"], "RGB(128,0,0)"],
					[["msg","expressing physical desire"], [255, 192, 203]]
				]
		    },
		    "response-short": {
		        "colors": [
					[128,0,0],
					[255,192,203]
		        ]
		    }
		})ML");
		json_input.AddUser(R"ML(
		{
		    "query": {
		        "script": {
		            "actions": [],
		            "__comment__": "get response-short"
		        }
		    }
		})ML")
			.Set("/query/script/actions", args.params("actions"));
		input.response_length = 2048;
	}
	else if (args.fn == FN_CLASSIFY_PHRASE_ACTION_ATTR) {
		Panic("TODO");
		#if 0
		R"ML({
			"query": {
				"__comment__": "Definition of attribute list with various attributes",
				"attributes": {
					"FAITH_AND_REASON_SEEKER": ["faith and reason seekers", "divine worshipers", "rational thinker"],
					"GROUP_FAITH": ["group faith", "individual spirituality", "organized religion"],
					"BELIF_SPECTRUM": ["belief spectrum", "believer", "non-believer"],
					"OLD_AND_NEW_BELIEVER": ["old and new believers", "new age spirituality", "traditional religion"],
					"BELIF_COMMUNITY": ["belief communities", "secular society", "religious community"],
					"THEOLOGICAL_OPPOSITE": ["theological opposites", "theistic", "atheistic"],
					"SEEKER_OF_TRUTH": ["seekers of truth", "spiritual seeker", "skeptic"],
					"INTUITIVE_THINKER": ["intuitive thinkers", "rationalist", "mystic practitioner"],
					"RATIONAL_BELIEF": ["rational believers", "religious", "scientific"],
					"PHYSICAL_PREFERENCE": ["physical preference", "body enhancing beauty", "natural beauty"],
					"SEXUAL_ORIENTATION": ["sexual orientation", "heterosexual", "homosexual"],
					"SEXUAL_PREFERENCE": ["sexual preference", "normal", "kinky"],
					"FAITH_EXTREME": ["faith extremes", "agnostic", "religious fundamentalist"],
					// Add more attributes as needed
				},
				"script": {
					"lyrics": [
						"2 AM, howlin outside",
						"Lookin, but I cannot find"
					],
					"actions_per_line": [
						"\"2 AM, howlin outside\": attention-time(night) + attention-emotional_state(desire) + attention-action(howling) + attention-activity(driving) + tone(urgent) + msg(trying to reach someone) + bias(romantic + emotion(uncertainty) + level-of-certainty(trying/desire) + gesturing(pointing) + describing-surroundings(anywhere in the dark) + attention-place(outside)",
						"\"Lookin, but I cannot find\": attention-action(looking) + attention-physical state(tired) + emotion(frustration) + attention-emotional_state(desperation) + attention-time(late at night)"
					],
					"attribute_list_A": [
						"faith and reason seekers / divine worshipers / rational thinker",
						"group faith / individual spirituality / organized religion",
						"belief spectrum / believer / non-believer",
						// Add more attribute list A items as needed
					],
					"primary_attributes": [
						"\"I won't blindly follow the crowd\": group faith / individual spirituality",
						"\"feeling blue and green with envy\": sexual preference / kinky",
						"\"2 AM, howlin outside\": faith and reason seekers / divine worshipers",
						"\"Lookin, but I cannot find\": truthfulness / personal experience"
					],
					"actions_list_C": [
						"\"attention-event(unpleasant smell)\"",
						"\"transition(activities/roles)\""
					],
					"additional_actions_C": [
						"\"attention-event(unpleasant smell)\": sexualization / non-sexual",
						"\"transition(activities/roles)\": integrity / twisted"
					]
				}
			}
		})ML";
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
		#endif
	}
	else if (args.fn == FN_SORT_ATTRS) {
		Panic("TODO");
		#if 0
		R"ML({
			"query": {
				"list_A_values": {
					"title": "List \"A\" values in the same group '" + args.group + "'",
					"values": [
						"Value 1",
						"Value 2",
						"Value 3"
					]
				},
				"results": {
					"summary_main_values": {
						"title": "2 main values of list \"A\", which summarizes all values in a way",
						"description": "The first value is the common attribute of modern pop/rock/edm songs, and the second value is the polar opposite of the first"
					},
					"sorted_values": {
						"title": "Sort 2 values of list \"A\" in a way",
						"description": "The first value is the one which is closer to a common attribute of modern pop/rock/edm songs. Use same values, but just sort the values. Don't add any text"
					}
				}
			}
		})ML";
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
		#endif
	}
	else if (args.fn == FN_ATTR_POLAR_OPPOSITES) {
		Panic("TODO");
		#if 0
		R"ML({
			"query": {
				"__assertions__": [
					"args.attr0.GetCount()",
					"args.attr1.GetCount()"
				],
				"tasks": [
					{
						"title": "List \"A\" values in the same group 'socioeconomic status'",
						"values": ["urban", "gang affiliation", "drug dealing"]
					},
					{
						"title": "List \"B\" polar opposites of the group 'socioeconomic status'",
						"values": ["positive: wealth", "negative: poverty"]
					},
					{
						"title": "Values of list \"A\", with their closest polar opposite value of list \"B\". Either 'positive' or 'negative'",
						"values": ["positive", "negative", "negative"]
					},
					{
						"title": "List \"C\" values in the same group '{{args.group}}'",
						"values": "[dynamic values here]"
					},
					{
						"title": "List \"D\" polar opposites of the group '{{args.group}}'",
						"values": ["positive: {{args.attr0}}", "negative: {{args.attr1}}"]
					}
				]
			},
			"response": {
				"results": {
					"title": "Values of list \"C\", with their closest polar opposite value of list \"D\". Either 'positive' or 'negative",
					"values": ["[dynamic values here]"]
				}
			}
		})ML";
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
		#endif
	}
	else if (args.fn == FN_MATCHING_ATTR) {
		Panic("TODO");
		#if 0
		R"ML({
			"query": {
				"__comment__": "Ensure necessary data present in args object",
				"attributes_groups": {
					"list_A": [
						"Attribute Group 1",
						"Attribute Group 2",
						"Attribute Group 3"
					]
				},
				"orphaned_groups_values": {
					"list_B": [
						"culture: mainstream success",
						"Value 1",
						"Value 2",
						"Value 3"
					]
				}
			},
			"response": {
				"mapping_results": {
					"title": "For the values of list \"B\", their closest matching group and polarised extreme value from list \"A\"",
					"results": {
						"1": {
							"value": "5 +",
							"mapping": ""
						},
						"2": {
							"value": "Value 1",
							"mapping": "Attribute Group 1: extreme value 1"
						},
						"3": {
							"value": "Value 2",
							"mapping": "Attribute Group 2: extreme value 2"
						},
						"4": {
							"value": "Value 3",
							"mapping": "Attribute Group 3: extreme value 3"
						}
					}
				}
			}
		})ML";
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
		#endif
	}
	else
		SetError("Invalid function");
}

void AiTask::CreateInput_GenericPrompt()
{
	if(args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}

	GenericPromptArgs args;
	args.Put(this->args[0]);
	
	{
		for(int i = 0; i < args.lists.GetCount(); i++) {
			String s = args.lists.GetKey(i);
			auto& list = input.AddSub().Title(s);
			const auto& arr = args.lists[i];
			if(arr.IsEmpty())
				continue;
			for(int j = 0; j < arr.GetCount(); j++) {
				list.Add(arr[j]);
			}
		}
		if (args.response_path.GetCount()) {
			auto& list = input.AddSub();
			list.Title(args.response_path);
			list.NoColon();
		}
		{
			TaskTitledList& results = input.PreAnswer();
			results.Title(args.response_title);
			if(args.is_numbered_lines)
				results.NumberedLines();
			results.Add("");
		}
		input.response_length = 2048;
	}
}

void AiTask::CreateInput_Code()
{
	if(args.IsEmpty()) {
		SetFatalError("no args");
		return;
	}
	CodeArgs args;
	args.Put(this->args[0]);
	
	Panic("TODO: convert to json");
	
	if(args.fn == CodeArgs::SCOPE_COMMENTS) {
		{
			auto& list = input.AddSub().Title("Code");
			for(int i = 0; i < args.code.GetCount(); i++) list.Add("line #" + IntStr(i) + ": " + args.code[i]);
			list.NoListChar();
		}
		{
			TaskTitledList& results = input.PreAnswer();
			//results.Title("Add the comments which explains the purpose of the code. List of comments to insert (without code & all text in single line)");
			results.Title("Add the comments to the " + args.lang + " code. List of comments to insert (without code & all text in single line)");
			if (args.lang == "C++")
				tmp_str = "line #0: //";
			else
				tmp_str = "line #0: \"";
			results.Add(tmp_str);
		}
		input.response_length = 2048;
	}
	if(args.fn == CodeArgs::FUNCTIONALITY) {
		#if 0
		{
			auto& list = input.AddSub().Title("List of types of functionalities");
			//list.NumberedLines();
			list.Add("generic");
			list.Add("writing data");
			list.Add("reading data");
			list.Add("referencing data for potential reading and writing");
			list.Add("referencing data for reading");
			list.Add("calling another function");
			list.Add("jumping unconditionally");
			list.Add("conditional branching");
			list.Add("synchronized (mutex, spinlock, etc.)");
			list.Add("asynchronized (callbacks, this-pointer, function/method pointers, etc.)");
			list.Add("etc.");
		}
		#endif
		{
			auto& list = input.AddSub().Title("Code");
			for(int i = 0; i < args.code.GetCount(); i++)
				list.Add(args.code[i]);
			list.NoListChar();
		}
		if (!args.data.IsEmpty()) {
			auto& list = input.AddSub().Title("Datapoints in Code");
			for(int i = 0; i < args.data.GetCount(); i++)
				list.Add(args.data.GetKey(i), args.data[i]);
		}
		{
			TaskTitledList& results = input.PreAnswer();
			//results.Title("List of functionalities and descriptions/explanations for the given code");
			results.Title("Explain the functionality/purpose for the given code");
			//tmp_str = "- type #";
			results.Add(tmp_str);
		}
		input.response_length = 2048;
	}
}

END_UPP_NAMESPACE

