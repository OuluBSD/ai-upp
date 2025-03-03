#ifndef _AI_TextCore_Defs_h_
#define _AI_TextCore_Defs_h_

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
	

#define TYPECAST_LIST \
	TYPECAST(1, "Creative/Innovative", "Emphasizing originality and new ideas.") \
	TYPECAST(2, "Efficient/Optimized", "Focused on performance and resourcefulness.") \
	TYPECAST(3, "Collaborative/Inclusive", "Encouraging teamwork and participation.") \
	TYPECAST(4, "Experimental/Adventurous", "Willingness to try new approaches.") \
	TYPECAST(5, "Reliable/Consistent", "Dependable and stable.") \
	TYPECAST(6, "Scalable/Expandable", "Able to grow and adapt to increasing demands.") \
	TYPECAST(7, "Immutable/Constant", "Unchanging and steady.") \
	TYPECAST(8, "Structured/Organized", "Well-planned and systematic.") \
	TYPECAST(9, "Legacy/Traditional", "Rooted in established practices.") \
	TYPECAST(10, "Pure/Simple", "Uncomplicated and straightforward.") \
	TYPECAST(11, "Modular/Component-Based", "Made up of distinct parts that fit together.") \
	TYPECAST(12, "Reflective/Introspective", "Thoughtful and self-examining.") \
	TYPECAST(13, "Dynamic/Flexible", "Able to adapt to change.") \
	TYPECAST(14, "Challenging/Complex", "Involving intricate or difficult elements.") \
	TYPECAST(15, "Basic/Foundational", "Core and essential elements.") \
	TYPECAST(16, "User-Friendly/Accessible", "Easy to understand and engage with.") \
	TYPECAST(17, "Performant/Fast", "Quick and efficient.") \
	TYPECAST(18, "Lightweight/Minimalist", "Simple and not overbearing.") \
	TYPECAST(19, "Mysterious/Enigmatic", "Puzzling and intriguing.") \
	TYPECAST(20, "Clear/Understandable", "Easy to comprehend.") \
	TYPECAST(21, "Controversial/Debated", "Provoking discussion or disagreement.") \
	TYPECAST(22, "Compatible/Integrable", "Able to work well with others.") \
	TYPECAST(23, "Logical/Rational", "Thought through and reasoned.") \
	TYPECAST(24, "Verbose/Descriptive", "Expressive and detailed.") \
	TYPECAST(25, "Cross-disciplinary", "Spanning multiple areas or fields.") \
	TYPECAST(26, "Stable/Secure", "Safe and consistent.") \
	TYPECAST(27, "Unstable/Fragile", "Prone to change or damage.") \
	TYPECAST(28, "Forward-Thinking/Future-Oriented", "Looking towards future developments.") \
	TYPECAST(29, "Intricate/Complicated", "Containing many elements and details.") \
	TYPECAST(30, "Unique/Unconventional", "Original and different.") \
	TYPECAST(31, "Sensitive/Responsive", "Quickly reacting to inputs or changes.") \
	TYPECAST(32, "Inefficient/Laborious", "Requiring more effort or resources than necessary.") \
	TYPECAST(33, "Customizable/Adaptable", "Easily modified to suit different needs.") \
	TYPECAST(34, "Portable/Transferrable", "Easily moved or applied in different contexts.") \
	TYPECAST(35, "Secure/Protected", "Safe and guarded.") \
	TYPECAST(36, "Standardized/Common", "Widely accepted or used.") \
	TYPECAST(37, "Authentic/Real", "Genuine and true to original form.") \
	TYPECAST(38, "Encapsulated/Contained", "Self-contained and separate.") \
	TYPECAST(39, "Spontaneous/Unplanned", "Arising without premeditation.") \
	TYPECAST(40, "Smooth/Effortless", "Operating with minimal difficulty.") \
	TYPECAST(41, "Abstract/Conceptual", "Not tied to concrete systems, existing in ideas.") \
	TYPECAST(42, "Supportive/Nurturing", "Encouraging and helpful.") \
	TYPECAST(43, "Historic/Old-School", "Reflecting old methods or styles.") \
	TYPECAST(44, "Resilient/Fault-Tolerant", "Able to withstand or recover from difficulties.") \
	TYPECAST(45, "Bold/Expressive", "Strong and vivid.") \
	TYPECAST(46, "Independent/Self-Sufficient", "Able to function autonomously.") \
	TYPECAST(47, "Disconnected/Detached", "Not connected or linked.") \
	TYPECAST(48, "Robust/Resilient", "Strong and able to handle change.") \
	TYPECAST(49, "Sincere/Genuine", "Honest and direct.") \
	TYPECAST(50, "Transparent/Open", "Clear and easy to understand or access.") \
	TYPECAST(51, "Simple/Easy", "Not complex and easily approachable.") \
	TYPECAST(52, "Risk-Taking/Daring", "Willing to take risks for potential gain.")


#define CONTENT_LIST \
	CONTENT(0, "Creative introduction") \
	CONTENT(1, "Progression and regression") \
	CONTENT(2, "Exploration and experimentation") \
	CONTENT(3, "Discovery and fascination") \
	CONTENT(4, "Challenge and victory") \
	CONTENT(5, "Fluctuations and fluctuations") \
	CONTENT(6, "Seeking solace and tranquility") \
	CONTENT(7, "Rebellion against norms") \
	CONTENT(8, "Breaking and rebuilding") \
	CONTENT(9, "Pursuing ambitions and aspirations") \
	CONTENT(10, "Unearthing hidden truths") \
	CONTENT(11, "Transformation from struggle to success") \
	CONTENT(12, "Rediscovery and renewal") \
	CONTENT(13, "Igniting passion and motivation") \
	CONTENT(14, "Emerging stronger from adversity") \
	CONTENT(15, "Recognition and prosperity") \
	CONTENT(16, "Finding resilience in hardship") \
	CONTENT(17, "Navigating urban chaos and solitude") \
	CONTENT(18, "Defying conventions and forging new paths") \
	CONTENT(19, "Haunted by ghosts of the past") \
	CONTENT(20, "Embracing freedom and spontaneity") \
	CONTENT(21, "Encountering conflicting viewpoints") \
	CONTENT(22, "Navigating distant connections") \
	CONTENT(23, "Harnessing inner strength and fortitude") \
	CONTENT(24, "Balancing dual identities") \
	CONTENT(25, "Thriving under the spotlight") \
	CONTENT(26, "Navigating love and conflict") \
	CONTENT(27, "Mastering the art of letting go") \
	CONTENT(28, "Living in the present moment") \
	CONTENT(29, "Overcoming fears and obstacles")


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
