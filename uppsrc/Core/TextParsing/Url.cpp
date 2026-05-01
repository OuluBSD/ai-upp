#include "TextParsing.h"

NAMESPACE_UPP

UrlParser::UrlParser() {
	
}

bool UrlParser::Parse(String url) {
	this->url = url;
	is_protocol_defined = false;
	
	// Tokenize
	tokens.Clear();
	for(int i = 0; i < url.GetCount(); i++) {
		int chr = url[i];
		if (chr == '/') {
			if (i + 1 < url.GetCount() && url[i+1] == '/') {
				tokens.Add().Set(DOUBLESLASH, "//");
				i++;
			} else {
				tokens.Add().Set(SLASH, "/");
			}
		} else if (chr == ':') {
			tokens.Add().Set(COLON, ":");
		} else {
			String text;
			while (i < url.GetCount() && url[i] != '/' && url[i] != ':') {
				text.Cat(url[i]);
				i++;
			}
			tokens.Add().Set(TEXT, text);
			i--;
		}
	}
	
	cursor = 0;
	ReadProtocol();
	
	without_protocol = "";
	for(int i = cursor; i < tokens.GetCount(); i++) {
		without_protocol += tokens[i].text;
	}
	
	// Base addr is everything before the last slash
	int i = url.ReverseFind("/");
	if (i >= 0)
		base_addr = url.Left(i+1);
	else
		base_addr = url;
		
	return true;
}

void UrlParser::ReadProtocol() {
	if (tokens.GetCount() >= 2 && tokens[0].type == TEXT && tokens[1].type == COLON) {
		protocol = tokens[0].text;
		cursor = 2;
		is_protocol_defined = true;
		if (cursor < tokens.GetCount() && tokens[cursor].type == DOUBLESLASH) {
			cursor++;
		}
	}
}

String UrlParser::GetFormattedAddr() const {
	return url;
}

String UrlParser::GetFormattedBaseAddr() const {
	return base_addr;
}

String UrlParser::GetFormattedDirectory() const {
	String addr = GetFormattedAddr();
	int i = addr.ReverseFind("/");
	if (i >= 0)
		return addr.Left(i+1);
	return addr;
}

String JoinUrl(String base, String rel_path) {
	while (base.Right(1) == "/") base = base.Left(base.GetCount() - 1);
	while (rel_path.Left(1) == "/") rel_path = rel_path.Mid(1);
	return base + "/" + rel_path;
}

END_UPP_NAMESPACE
