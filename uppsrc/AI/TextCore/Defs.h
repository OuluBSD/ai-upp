#ifndef _AI_TextCore_Defs_h_
#define _AI_TextCore_Defs_h_

#define TODO Panic("TODO");

#define DBROWSER_MODE_LIST \
	MODE(ELEMENT_ATTR_COLOR_ACTION) \
	MODE(ELEMENT_COLOR_ATTR_ACTION) \
	MODE(ELEMENT_COLOR_CONTENT_TYPECLASS) \
	MODE(ELEMENT_COLOR_TYPECLASS_CONTENT) \
	MODE(ATTR_COLOR_ACTION) \
	MODE(ATTR_ACTION_COLOR) \
	MODE(COLOR_ELEMENT_ATTR_ACTION) \
	MODE(COLOR_ACTION_ATTR) \
	MODE(COLOR_ATTR_ACTION) \
	MODE(ACTION_COLOR_ATTR) \
	MODE(ACTION_ATTR_COLOR) \
	MODE(TYPECLASS_CONTENT_COLOR) \
	MODE(TYPECLASS_COLOR_CONTENT) \
	MODE(CONTENT_TYPECLASS_COLOR) \
	MODE(CONTENT_COLOR_TYPECLASS) \
	MODE(COLOR_CONTENT_TYPECLASS) \

#define INHIBIT_CURSOR(x) CallbackInhibitor __cur(x.WhenCursor)
#define INHIBIT_CURSOR_(x, id) CallbackInhibitor __##id(x.WhenCursor)
#define INHIBIT_ACTION(x) CallbackInhibitor __act(x.WhenAction)
#define INHIBIT_ACTION_(x, id) CallbackInhibitor __##id(x.WhenAction)



// These are sorted using AI (14.10.2023). The first one is the popular option

#define ATTR_LIST \
	ATTR_ITEM(FAITH_AND_REASON_SEEKER, "faith and reason seekers", "divine worshipers", "rational thinker") \
	ATTR_ITEM(GROUP_FAITH, "group faith", "individual spirituality", "organized religion") \
	ATTR_ITEM(BELIF_SPECTRUM, "belief spectrum", "believer", "non-believer") \
	ATTR_ITEM(OLD_AND_NEW_BELIEVER, "old and new believers", "new age spirituality", "traditional religion") \
	ATTR_ITEM(BELIF_COMMUNITY, "belief communities", "secular society", "religious community") \
	ATTR_ITEM(THEOLOGICAL_OPPOSITE, "theological opposites", "theistic", "atheistic") \
	ATTR_ITEM(SEEKER_OF_TRUTH, "seekers of truth", "spiritual seeker", "skeptic") \
	ATTR_ITEM(INTUITIVE_THINKER, "intuitive thinkers", "rationalist", "mystic practitioner") \
	ATTR_ITEM(RATIONAL_BELIEF, "rational believers", "religious", "scientific") \
	ATTR_ITEM(PHYSICAL_PREFERENCE, "physical preference", "body enhancing beauty", "natural beauty") \
	ATTR_ITEM(SEXUAL_ORIENTATION, "sexual orientation", "heterosexual", "homosexual") \
	ATTR_ITEM(SEXUAL_PREFERENCE, "sexual preference", "normal", "kinky") \
	ATTR_ITEM(FAITH_EXTREME, "faith extremes", "agnostic", "religious fundamentalist") \
	\
	ATTR_ITEM(AVERAGE_EXPECTATIONS, "average expectations", "expectation-conformed", "expectation-opposed") \
	ATTR_ITEM(IMAGERY, "imagery", "trope-reinforcing", "unique") \
	ATTR_ITEM(EXPRESSION, "expression", "explicit", "allegorical") \
	ATTR_ITEM(RELATIONSHIP, "relationship", "romantic couple", "without romantic partner" ) \
	ATTR_ITEM(RELATIONSHIP_FOCUS, "relationship focus", "partner-focused", "individual-focused") \
	ATTR_ITEM(HUMAN_STRENGTH, "human strength", "strong", "weak") \
	ATTR_ITEM(GENDER, "gender", "female", "male") \
	ATTR_ITEM(RATIONALITY, "rationality", "unreasonable", "reasonable") \
	ATTR_ITEM(INTEGRITY, "integrity", "twisted", "honest") \
	ATTR_ITEM(SEXUALIZATION, "sexualization", "sexual", "non-sexual") \
	ATTR_ITEM(EXPECTATIONS, "expectations", "perfection", "acceptance") \
	ATTR_ITEM(PROBLEM_SOLVING, "problem solving strategy", "shortcut taking", "cunning") \
	ATTR_ITEM(RESPONSIBILITY, "responsibility", "irresponsible", "accountable") \
	ATTR_ITEM(SOCIAL, "social", "authoritarian", "libertarian") \
	ATTR_ITEM(ECONOMIC, "economic", "liberal", "conservative") \
	ATTR_ITEM(CULTURE, "culture", "individualistic", "collective") \
	ATTR_ITEM(GROUP_EXPERIENCE, "group experience", "group-oriented", "individual-oriented") \
	ATTR_ITEM(MOTIVATION, "motivation", "rewarding", "punishing") \
	ATTR_ITEM(LOVE_STATUS, "love status", "loving now", "heartbreak") \
	ATTR_ITEM(SITUATION_RELATION, "situation relation", "prescriptive", "descriptive") \
	ATTR_ITEM(COMPETITIVENESS, "competitiveness", "competition", "collaboration") \
	ATTR_ITEM(OTHER_RELATION, "relation to others", "comparison", "self-acceptance") \
	ATTR_ITEM(INTELLIGENCE, "intelligence", "emotional", "intellectual") \
	ATTR_ITEM(SOPHISTICATION, "sophistication", "sophisticated", "simple") \
	ATTR_ITEM(TRANSPARENCY, "transparency", "transparent", "mysterious") \
	\
	ATTR_ITEM(STORYMODE, "storymode", "storytelling", "simple themes") \
	ATTR_ITEM(TRUTHFULNESS, "truthfulness", "personal experience", "fictional") \
	ATTR_ITEM(MINDFULNESS, "mindfulness", "mindless", "mindful") \
	ATTR_ITEM(PEACEFULNESS, "peacefulness", "peacemaker", "troublemaker") \
	ATTR_ITEM(NARRATIVE, "narrative", "protagonist storytelling", "narrative detachment") \
	ATTR_ITEM(LYRICAL_EMPHASIS, "lyrical emphasis", "witty wordplay", "straightforward scripts") \
	ATTR_ITEM(EMOTIONALITY, "lyrical emphasis", "emotionally charged", "emotionally restrained") \
	ATTR_ITEM(CONCEPTS, "concepts", "grounded", "psychedelic") \
	ATTR_ITEM(STORY_MOTIVATION, "story-motivation",  "narrative-driven", "mood-driven") \
	ATTR_ITEM(REALITY, "reality", "escapism", "realism") \
	ATTR_ITEM(PROVOCATIVITY, "provocativity", "edgy", "innocent") \
	ATTR_ITEM(INSPIRATION_SOURCE, "source of inspiration", "nostalgic", "relevant to current times") \
	ATTR_ITEM(PRODUCTION_STYLE, "production style", "acoustic", "electronic") \
	ATTR_ITEM(LIFE_AND_LOVE, "life and love", "aspirational", "relatable") \
	ATTR_ITEM(AUDIENCE, "audience", "mainstream", "alternative") \
	ATTR_ITEM(AUTHENCITY, "authencity", "social media-driven", "authentic") \
	ATTR_ITEM(PATIENCE, "patience", "instant gratification", "longevity") \
	ATTR_ITEM(LOVE_TARGET, "target of love", "romantic love", "self-love") \
	ATTR_ITEM(AGE_TARGET, "age target", "youth-oriented", "age-inclusive") \
	ATTR_ITEM(INDEPENDENCE, "independence", "independence", "dependence") \
	ATTR_ITEM(COMMERCIAL_APPEAL, "commercial appeal", "mainstream success", "artistic integrity") \
	ATTR_ITEM(CULTURAL_ELEMENTS, "cultural elements", "globalization", "cultural preservation") \
	ATTR_ITEM(CORPORATION_RELATIONS, "corporation relations", "consumerism", "anti-capitalism") \
	ATTR_ITEM(CELEBRITY_PERSONA, "celebrity Persona", "celebrity worship", "body positivity") \
	ATTR_ITEM(EMOTIONAL_REALISM, "emotional realism",  "happiness", "realistic emotions") \
	ATTR_ITEM(PARTY_RELATION, "party relation", "party anthems", "introspective ballads") \
	ATTR_ITEM(ATTITUDE_ATTITUDE_OPEN_CLOSED, "attitude 1", "open", "closed") \
	ATTR_ITEM(ATTITUDE_HOPEFUL_DESPAIR, "attitude 2", "hopeful", "despair") \
	ATTR_ITEM(ATTITUDE_OPTIMISTIC_PESSIMISTIC, "attitude 3", "optimistic", "pessimistic") \
	ATTR_ITEM(SEXUALITY, "sexuality", "adventurous", "limited") \
	ATTR_ITEM(SEXUAL_ACTING, "sexual acting", "confident", "sensitive") \
	ATTR_ITEM(SEXUAL_COMMITMENT, "sexual commitment", "monogamy", "polygamy") \
	ATTR_ITEM(MOOD_JOYFUL_MELANCHOLIC, "mood 1", "joyful", "melancholic") \
	ATTR_ITEM(MOOD_LIGHTHEARTED_SOMBER, "mood 2", "lighthearted", "somber") \
	ATTR_ITEM(MOOD_UPLIFTING_HEAVY, "mood 3", "uplifting", "heavy") \
	ATTR_ITEM(MOOD_HUMOROUS_DRAMATIC, "mood 4", "humorous", "dramatic") \
	ATTR_ITEM(MOOD_PLAYFUL_SERIOUS, "mood 5", "playful", "serious") \
	


struct CallbackInhibitor {
	Event<> cb;
	Event<>& ref;
	
	CallbackInhibitor(Event<>& other) : cb(other), ref(other) {other.Clear();}
	~CallbackInhibitor() {ref = cb;}
};

#define INHIBIT_CURSOR(x) CallbackInhibitor __cur(x.WhenCursor)
#define INHIBIT_CURSOR_(x, id) CallbackInhibitor __##id(x.WhenCursor)
#define INHIBIT_ACTION(x) CallbackInhibitor __act(x.WhenAction)
#define INHIBIT_ACTION_(x, id) CallbackInhibitor __##id(x.WhenAction)



#endif
