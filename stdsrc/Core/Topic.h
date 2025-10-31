#pragma once
#ifndef _Core_Topic_h_
#define _Core_Topic_h_

#include <string>
#include <vector>
#include <map>
#include "Core.h"

struct Topic : Moveable<Topic> {
	String title;
	String text;
	String link;
	String label;

	operator const String&() const { return text; }
	operator const char *() const  { return text; }
};

struct TopicLink {
	String package;
	String group;
	String topic;
	String label;

	operator bool() const { return !IsNull(topic); }
};

String     TopicLinkString(const TopicLink& tl);
TopicLink  ParseTopicLink(const char *link);

Topic      GetTopic(const String& package, const String& group, const String& topic);
Topic      GetTopic(const char *path);
Topic      GetTopicLNG(const char *path);

VectorMap<String, VectorMap<String, Vector<String> > >& TopicBase();

// Implementations
inline String TopicLinkString(const TopicLink& tl) {
	if (tl.package.IsEmpty() && tl.group.IsEmpty() && tl.topic.IsEmpty()) {
		return String();
	}
	return tl.package + "::" + tl.group + "::" + tl.topic;
}

inline TopicLink ParseTopicLink(const char *link) {
	TopicLink result;
	if (!link) return result;
	
	String s = link;
	Vector<String> parts = Split(s, "::");
	if (parts.GetCount() >= 3) {
		result.package = parts[0];
		result.group = parts[1];
		result.topic = parts[2];
		if (parts.GetCount() >= 4) {
			result.label = parts[3];
		}
	}
	return result;
}

// Stub implementations for now
inline Topic GetTopic(const String& package, const String& group, const String& topic) {
	Topic t;
	t.title = package + " " + group + " " + topic;
	t.text = "Topic text for " + package + " " + group + " " + topic;
	t.link = package + "::" + group + "::" + topic;
	return t;
}

inline Topic GetTopic(const char *path) {
	Topic t;
	t.title = path;
	t.text = "Topic text for " + String(path);
	t.link = path;
	return t;
}

inline Topic GetTopicLNG(const char *path) {
	return GetTopic(path);
}

inline VectorMap<String, VectorMap<String, Vector<String> > >& TopicBase() {
	static VectorMap<String, VectorMap<String, Vector<String>>> base;
	return base;
}

#endif