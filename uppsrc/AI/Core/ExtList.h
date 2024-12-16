#ifndef _AI_Core_ExtList_h_
#define _AI_Core_ExtList_h_

#define NODE_LIST \
	DATASET_ITEM(MetaNode,			env,			METAKIND_PKG_ENV,							CATEGORY_ECS,"Environment") \
	DATASET_ITEM(MetaNode,			ctx,			METAKIND_CONTEXT,							CATEGORY_ECS,"Context") \
	DATASET_ITEM(MetaNode,			dbref,			METAKIND_DB_REF,							CATEGORY_ECS,"Database Reference") \
	DATASET_ITEM(MetaNode,			comment,		METAKIND_COMMENT,							CATEGORY_ECS,"Comment") \
	DATASET_ITEM(MetaNode,			space,			METAKIND_ECS_SPACE,							CATEGORY_ECS,"Space") \

#define EXT_LIST \
	DATASET_ITEM(Entity,			entity,			METAKIND_ECS_ENTITY,						CATEGORY_ECS, "Entity") \
	DATASET_ITEM(Component,			component,		METAKIND_ECS_COMPONENT_UNDEFINED,			CATEGORY_ECS, "Undefined Component") \

#define COMPONENT_LIST \
	DATASET_ITEM(ProjectWizardView,	prjwiz,			METAKIND_ECS_COMPONENT_PROJECT_WIZARD,		CATEGORY_ASSET, "Code Project Wizard") \
	DATASET_ITEM(SrcTxtHeader,		src,			METAKIND_DATABASE_SOURCE,					CATEGORY_DISPOSABLE, "Source Database") \
	\
	DATASET_ITEM(Owner,					owner,				METAKIND_ECS_COMPONENT_OWNER,						CATEGORY_PRIVATE, "Owner") /* TODO rename to human? */ \
	DATASET_ITEM(Notepad,				notepad,			METAKIND_ECS_COMPONENT_NOTEPAD,						CATEGORY_PRIVATE, "Notepad") \
	DATASET_ITEM(Biography,				biography,			METAKIND_ECS_COMPONENT_BIOGRAPHY,					CATEGORY_PRIVATE, "Biography") \
	DATASET_ITEM(BiographyAnalysis,		analysis,			METAKIND_ECS_COMPONENT_BIOGRAPHY_ANALYSIS,			CATEGORY_PRIVATE, "Biography Analysis") \
	DATASET_ITEM(BiographyElements,		biography_elem,		METAKIND_ECS_COMPONENT_BIOGRAPHY_ELEMENTS,			CATEGORY_PRIVATE, "Biography Elements") \
	DATASET_ITEM(BiographySummary,		biography_sum,		METAKIND_ECS_COMPONENT_BIOGRAPHY_SUMMARY,			CATEGORY_PRIVATE, "Biography Summary") \
	DATASET_ITEM(BiographyPerspectives,	snap,				METAKIND_ECS_COMPONENT_BIOGRAPHY_PERSPECTIVES,		CATEGORY_PRIVATE, "Biography Perspectives") \
	DATASET_ITEM(ImageBiography,		biography_img,		METAKIND_ECS_COMPONENT_BIOGRAPHY_IMAGES,			CATEGORY_PRIVATE, "Biography Images") \
	DATASET_ITEM(ImageBiographySummary,	biography_imgsum,	METAKIND_ECS_COMPONENT_BIOGRAPHY_IMAGES_SUMMARY,	CATEGORY_PRIVATE, "Biography Images Summary") \
	DATASET_ITEM(Profile,				profile,			METAKIND_ECS_COMPONENT_PROFILE,						CATEGORY_PUBLIC, "Profile") \
	DATASET_ITEM(Release,				release,			METAKIND_ECS_COMPONENT_RELEASE,						CATEGORY_PUBLIC, "Release") \
	DATASET_ITEM(Perspective,			perspective,		METAKIND_ECS_COMPONENT_PERSPECTIVE,					CATEGORY_PUBLIC, "Perspective") \
	DATASET_ITEM(Artist,				artist,				METAKIND_ECS_COMPONENT_ARTIST,						CATEGORY_PUBLIC, "Artist") \
	DATASET_ITEM(ReleaseBriefing,		rel_brief,			METAKIND_ECS_COMPONENT_RELEASE_BRIEFING,			CATEGORY_PUBLIC, "Release Briefing") \
	DATASET_ITEM(ReleaseCoverImage,		rel_cover_img,		METAKIND_ECS_COMPONENT_RELEASE_COVER_IMAGE,			CATEGORY_PUBLIC, "Release Cover Image") \
	DATASET_ITEM(Audience,				audience,			METAKIND_ECS_COMPONENT_AUDIENCE,					CATEGORY_PUBLIC, "Audience") \
	DATASET_ITEM(SocialHeader,			social_header,		METAKIND_ECS_COMPONENT_SOCIAL_HEADER,				CATEGORY_PUBLIC, "Header") \
	DATASET_ITEM(SocialContent,			social_content,		METAKIND_ECS_COMPONENT_SOCIAL_CONTENT,				CATEGORY_PUBLIC, "Messages") \
	DATASET_ITEM(SocialNeeds,			social_needs,		METAKIND_ECS_COMPONENT_SOCIAL_NEEDS,				CATEGORY_PUBLIC, "Needs") \
	DATASET_ITEM(Platform,				platform,			METAKIND_ECS_COMPONENT_PLATFORM,					CATEGORY_PUBLIC, "Platform") \
	DATASET_ITEM(PlatformProfile,		platform_prof,		METAKIND_ECS_COMPONENT_PLATFORM_PROFILE,			CATEGORY_PUBLIC, "Platform Profile") \
	\
	DATASET_ITEM(Provider,			provider,		METAKIND_ECS_COMPONENT_PROVIDER,			CATEGORY_MALE, "Provider") \
	DATASET_ITEM(DecisionMaker,		decision_maker,	METAKIND_ECS_COMPONENT_DECISION_MAKER,		CATEGORY_MALE, "Decision Maker") \
	DATASET_ITEM(Assertions,		assertions	,	METAKIND_ECS_COMPONENT_ASSERTIONS,			CATEGORY_MALE, "Assertions") \
	DATASET_ITEM(Aggressions,		aggressions,	METAKIND_ECS_COMPONENT_AGGRESSIONS,			CATEGORY_MALE, "Aggressions") \
	DATASET_ITEM(Strength,			strength,		METAKIND_ECS_COMPONENT_STRENGTH,			CATEGORY_MALE, "Strength") \
	DATASET_ITEM(Leadership,		leadership,		METAKIND_ECS_COMPONENT_LEADERSHIP,			CATEGORY_MALE, "Leadership") \
	DATASET_ITEM(Justice,			justice,		METAKIND_ECS_COMPONENT_JUSTICE,				CATEGORY_MALE, "Justice") \
	DATASET_ITEM(Sentimentality,	sentimentality,	METAKIND_ECS_COMPONENT_SENTIMENTALITY,		CATEGORY_FEMALE, "Sentimentality") \
	DATASET_ITEM(Addiction,			addiction,		METAKIND_ECS_COMPONENT_ADDICTION,			CATEGORY_FEMALE, "Addiction") \
	DATASET_ITEM(Caretaker,			caretaker,		METAKIND_ECS_COMPONENT_CARETAKER,			CATEGORY_FEMALE, "Caretaker") \
	DATASET_ITEM(SupportSystem,		support_sys,	METAKIND_ECS_COMPONENT_SUPPORT_SYSTEM,		CATEGORY_FEMALE, "Support System") \
	DATASET_ITEM(BeautyStandard,	beauty_std,		METAKIND_ECS_COMPONENT_BEAUTY_STANDARD,		CATEGORY_FEMALE, "Beauty Standard") \
	DATASET_ITEM(Attractiveness,	attractiveness,	METAKIND_ECS_COMPONENT_ATTRACTIVENESS,		CATEGORY_FEMALE, "Attractiveness") \
	DATASET_ITEM(Willingness,		willingness,	METAKIND_ECS_COMPONENT_WILLINGNESS,			CATEGORY_FEMALE, "Willingness") \
	\
	DATASET_ITEM(Marketplace,		marketplace,	METAKIND_ECS_COMPONENT_MARKETPLACE,			CATEGORY_BUYER, "Marketplace") \
	DATASET_ITEM(FactoryComponent,	factory,		METAKIND_ECS_COMPONENT_FACTORY,				CATEGORY_SELLER, "Factory") \
	DATASET_ITEM(ProductComponent,	product,		METAKIND_ECS_COMPONENT_PRODUCT,				CATEGORY_SELLER, "Product") \
	\
	DATASET_ITEM(ConsumerComponent,	consumer,		METAKIND_ECS_COMPONENT_CONSUMER,			CATEGORY_CONSUMER, "Consumer") \
	DATASET_ITEM(LeadData,			lead_data,		METAKIND_ECS_COMPONENT_LEAD_DATA,			CATEGORY_MARKETER, "Lead Data") \
	DATASET_ITEM(LeadDataTemplate,	lead_tmpl,		METAKIND_ECS_COMPONENT_LEAD_TEMPLATE,		CATEGORY_MARKETER, "Lead Template") \
	DATASET_ITEM(LeadDataPublisher,	ld_pub,			METAKIND_ECS_COMPONENT_LEAD_PUBLISHER,		CATEGORY_MARKETER, "Lead Publisher") \
	\
	DATASET_ITEM(CompositionComponent,	composition,	METAKIND_ECS_COMPONENT_COMPOSITION,			CATEGORY_MUSIC, "Composition") \
	DATASET_ITEM(LyricalStructure,		lyric_struct,	METAKIND_ECS_COMPONENT_SONG_IDEA,			CATEGORY_MUSIC, "Song Idea") \
	DATASET_ITEM(Script,				script,			METAKIND_ECS_COMPONENT_SCRIPT,				CATEGORY_TEXT, "Lyrics Draft") /* TODO rename to lyrics_draft */ \
	DATASET_ITEM(Lyrics,				lyrics,			METAKIND_ECS_COMPONENT_LYRICS,				CATEGORY_TEXT, "Lyrics") \
	DATASET_ITEM(Song,					song,			METAKIND_ECS_COMPONENT_SONG,				CATEGORY_TEXT, "Song") \
	DATASET_ITEM(ScriptReasoning,		script_reason,	METAKIND_ECS_COMPONENT_SCRIPT_REASONING,	CATEGORY_TEXT, "Script Reasoning") \
	\
	DATASET_ITEM(ImageLayer,		img_layer,				METAKIND_ECS_COMPONENT_IMG_LAYER,				CATEGORY_PHOTO, "Image layer") \
	DATASET_ITEM(ImageGenLayer,		img_gen_layer,			METAKIND_ECS_COMPONENT_IMG_GEN_LAYER,			CATEGORY_PHOTO, "Generate Image Layer") \
	DATASET_ITEM(AspectFixerLayer,	aspect_fixer,			METAKIND_ECS_COMPONENT_IMG_ASPECT_FIXER_LAYER,	CATEGORY_PHOTO, "Aspect Fix Image Layer") \
	DATASET_ITEM(VideoPromptMaker,	video_prompt_header,	METAKIND_ECS_COMPONENT_VIDEO_PROMPT_MAKER,		CATEGORY_VIDEO, "Video Prompt Maker") \
	DATASET_ITEM(VideoStoryboard,	vid_storyboard,			METAKIND_ECS_COMPONENT_VIDEO_STORYBOARD,		CATEGORY_VIDEO, "Video Storyboard") \

#define DATASET_LIST \
	NODE_LIST \
	EXT_LIST \
	COMPONENT_LIST \

#endif
