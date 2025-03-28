#include "Sound.h"

NAMESPACE_UPP

void SoundPhrase::Finish() {
	owner.owner.owner.OnPhraseEnd(*this);
}

void SoundMessage::Finish() {
	owner.owner.OnMessageEnd(*this);
}

void SoundDiscussion::Finish() {
	owner.OnDiscussionEnd(*this);
}




SoundPhrase& SoundMessage::Add() {
	return phrases.Add(new SoundPhrase(*this));
}

SoundMessage& SoundDiscussion::Add() {
	return messages.Add(new SoundMessage(*this));
}

SoundDiscussion& DiscussionManager::Add() {
	return discussions.Add(new SoundDiscussion(*this));
}

END_UPP_NAMESPACE
