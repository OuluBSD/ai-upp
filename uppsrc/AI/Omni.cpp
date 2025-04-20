#include "AI.h"


NAMESPACE_UPP

OmniThread::OmniThread() {
	
}

void OmniThread::Visit(NodeVisitor& vis) {
	vis	.VisitT("CompletionThread", (CompletionThread&)*this)
		.VisitT("ChatThread", (ChatThread&)*this)
		.VisitT("SpeechTranscriptionThread", (SpeechTranscriptionThread&)*this)
		.VisitT("SpeechGenerationThread", (SpeechGenerationThread&)*this)
		.VisitT("ImageGenerationThread", (ImageGenerationThread&)*this)
		.VisitT("ImageVisionThread", (ImageVisionThread&)*this)
		.VisitT("MetaEnvThread", (MetaEnvThread&)*this)
	;
}

void OmniThread::OnPhraseEnd(SoundPhrase& p) {
	LOG("TODO");
	
	// get transcript
}

void OmniThread::OnMessageEnd(SoundMessage&) {
	TODO
	
	// get proofread
}

void OmniThread::OnDiscussionEnd(SoundDiscussion&) {
	TODO
}

INITIALIZER_COMPONENT(StageThread)


INITIALIZER_COMPONENT(ChainThread)

END_UPP_NAMESPACE
