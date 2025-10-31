#pragma once
#ifndef _Core_Parser_h_
#define _Core_Parser_h_

#include <string>
#include <vector>
#include <functional>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "Core.h"

inline bool iscib(int c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c == '$';
}

inline bool iscid(int c) {
	return iscib(c) || c >= '0' && c <= '9';
}

class CParser {
private:
    const char *term;
    const char *wspc;
    const char *lineptr;
    int         line;
    String      fn;
    bool        skipspaces;
    bool        skipcomments;
    bool        nestcomments;
    bool        uescape;
    
protected:
	bool        Spaces0() {
		const char *s = term;
		lineptr = s;
		while(*s && (byte)*s <= ' ')
			if(*s++ == '\n')
				line++;
		if(skipcomments && *s == '/' && (s[1] == '/' || s[1] == '*')) {
			if(s[1] == '/') {
				s += 2;
				while(*s && *s != '\n')
					s++;
				if(*s == '\n') {
					s++;
					line++;
				}
				lineptr = s;
				term = s;
				return true;
			}
			else {
				s += 2;
				const char *begin = s;
				while(*s) {
					if(*s == '\n')
						line++;
					if(*s == '*' && s[1] == '/') {
						s += 2;
						break;
					}
					s++;
				}
				if(nestcomments && *s) {
					while(*s) {
						while(*s && *s != '/' && (s - begin < 1000000))
							if(*s++ == '\n')
								line++;
						if(s - begin > 1000000) break;
						if(*s == '/' && s[1] == '*') {
							s += 2;
							int q = 1;
							while(*s && q) {
								if(*s == '\n')
									line++;
								if(*s == '/' && s[1] == '*')
									q++;
								else
								if(*s == '*' && s[1] == '/')
									q--;
								s++;
							}
						}
						else
						if(*s == '/' && s[1] == '/')
							while(*s && *s++ != '\n')
								if(*s == '\n')
									line++;
						else
							break;
					}
				}
				lineptr = s;
				term = s;
				return true;
			}
		}
		lineptr = s;
		term = s;
		return false;
	}
	
	const char *IsId0(const char *s) const {
		const char *ptr = term;
		while(*s && *ptr == *s)
			s++, ptr++;
		return *s ? NULL : ptr;
	}
	
	bool Id0(const char *id) {
		const char *q = IsId0(id);
		if(q) {
			term = q;
			DoSpaces();
			return true;
		}
		return false;
	}
	
	void        DoSpaces()                    { if(skipspaces) Spaces(); }
	
	dword       ReadHex() {
		dword h = 0;
		while(*term) {
			int c = *term;
			if(c >= '0' && c <= '9')
				h = h * 16 + c - '0';
			else
			if(c >= 'A' && c <= 'F')
				h = h * 16 + c - 'A' + 10;
			else
			if(c >= 'a' && c <= 'f')
				h = h * 16 + c - 'a' + 10;
			else
				break;
			term++;
		}
		return h;
	}
	
	bool        ReadHex(dword& hex, int n) {
		const char *start = term;
		hex = 0;
		for(int i = 0; i < n; i++) {
			int c = *term;
			if(c >= '0' && c <= '9')
				hex = hex * 16 + c - '0';
			else
			if(c >= 'A' && c <= 'F')
				hex = hex * 16 + c - 'A' + 10;
			else
			if(c >= 'a' && c <= 'f')
				hex = hex * 16 + c - 'a' + 10;
			else
				return false;
			term++;
		}
		return true;
	}

public:
	struct Error : public Exc                 { Error(const char *s) : Exc(s) {} };

	void   ThrowError(const char *s) {
		String error_msg = String().Cat() << "Parser error at line " << line << ": " << s;
		throw Error(error_msg);
	}
	
	void   ThrowError()                       { ThrowError(""); }

	bool   Spaces() {
		wspc = term;
		return ((byte)*term <= ' ' || *term == '/') && Spaces0();
	}
	
	char   GetChar() {
		char c = *term;
		if(c) {
			term++;
			DoSpaces();
		}
		return c;
	}

	bool   IsChar(char c) const               { return *term == c; }
	bool   IsChar2(char c1, char c2) const    { return term[0] == c1 && term[1] == c2; }
	bool   IsChar3(char c1, char c2, char c3) const { return term[0] == c1 && term[1] == c2 && term[2] == c3; }
	bool   Char(char c);
	bool   Char2(char c1, char c2);
	bool   Char3(char c1, char c2, char c3);
	void   PassChar(char c) {
		if(!Char(c))
			ThrowError("Character expected");
	}
	void   PassChar2(char c1, char c2) {
		if(!Char2(c1, c2))
			ThrowError("Character sequence expected");
	}
	void   PassChar3(char c1, char c2, char c3) {
		if(!Char3(c1, c2, c3))
			ThrowError("Character sequence expected");
	}
	bool   Id(const char *s)                  { return term[0] == s[0] && (s[1] == 0 || term[1] == s[1]) && Id0(s); }
	void   PassId(const char *s) {
		if(!Id(s))
			ThrowError("Identifier expected");
	}
	bool   IsId() const                       { return iscib(*term); }
	bool   IsId(const char *s) const          { return term[0] == s[0] && (s[1] == 0 || term[1] == s[1]) && IsId0(s); }
	
	String ReadId() {
		const char *begin = term;
		while(iscid(*term))
			term++;
		String id = String(begin, (int)(term - begin));
		DoSpaces();
		return id;
	}
	
	String ReadIdh() {
		const char *begin = term;
		if(iscib(*term) || *term == '#') {
			term++;
			while(iscid(*term))
				term++;
		}
		String id = String(begin, (int)(term - begin));
		DoSpaces();
		return id;
	}
	
	String ReadIdt() {
		const char *begin = term;
		if(iscib(*term) || *term == '~') {
			term++;
			while(iscid(*term))
				term++;
		}
		String id = String(begin, (int)(term - begin));
		DoSpaces();
		return id;
	}
	
	bool   IsInt() const {
		const char *s = term;
		if(*s == '+' || *s == '-')
			s++;
		return IsDigit(*s);
	}
	
	int    Sgn() {
		if(Char('+'))
			return 1;
		if(Char('-'))
			return -1;
		return 1;
	}
	
	int    ReadInt() {
		int sgn = Sgn();
		uint32 n = 0;
		while(IsDigit(*term)) {
			int c = *term - '0';
			if(n > (UINT32_MAX - c) / 10)
				ThrowError("Number too big");
			n = n * 10 + c;
			term++;
		}
		DoSpaces();
		return sgn * (int)n;
	}
	
	int    ReadInt(int min, int max) {
		int n = ReadInt();
		if(n < min || n > max)
			ThrowError("Number out of range");
		return n;
	}
	
	int64  ReadInt64() {
		int sgn = Sgn();
		uint64 n = 0;
		while(IsDigit(*term)) {
			int c = *term - '0';
			if(n > (UINT64_MAX - c) / 10)
				ThrowError("Number too big");
			n = n * 10 + c;
			term++;
		}
		DoSpaces();
		return sgn * (int64)n;
	}
	
	int64  ReadInt64(int64 min, int64 max) {
		int64 n = ReadInt64();
		if(n < min || n > max)
			ThrowError("Number out of range");
		return n;
	}
	
	bool   IsNumber() const                   { return IsDigit(*term); }
	bool   IsNumber(int base) const {
		const char *s = term;
		if(*s == '+' || *s == '-')
			s++;
		if(base < 2 || base > 36)
			return false;
		while(true) {
			int c = *s;
			if(c >= '0' && c <= '9')
				c = c - '0';
			else if(c >= 'A' && c <= 'Z')
				c = c - 'A' + 10;
			else if(c >= 'a' && c <= 'z')
				c = c - 'a' + 10;
			else
				break;
			if(c >= base)
				break;
			s++;
		}
		return s > term;
	}
	
	uint32 ReadNumber(int base = 10) {
		if(base < 2 || base > 36) {
			ThrowError("Invalid base");
			return 0;
		}
		int sgn = Sgn();
		uint32 n = 0;
		while(true) {
			int c = *term;
			if(c >= '0' && c <= '9')
				c = c - '0';
			else if(c >= 'A' && c <= 'Z')
				c = c - 'A' + 10;
			else if(c >= 'a' && c <= 'z')
				c = c - 'a' + 10;
			else
				break;
			if(c >= base)
				break;
			if(n > (UINT32_MAX - c) / base)
				ThrowError("Number too big");
			n = n * base + c;
			term++;
		}
		DoSpaces();
		return sgn * n;
	}
	
	uint64 ReadNumber64(int base = 10) {
		if(base < 2 || base > 36) {
			ThrowError("Invalid base");
			return 0;
		}
		int sgn = Sgn();
		uint64 n = 0;
		while(true) {
			int c = *term;
			if(c >= '0' && c <= '9')
				c = c - '0';
			else if(c >= 'A' && c <= 'Z')
				c = c - 'A' + 10;
			else if(c >= 'a' && c <= 'z')
				c = c - 'a' + 10;
			else
				break;
			if(c >= base)
				break;
			if(n > (UINT64_MAX - c) / base)
				ThrowError("Number too big");
			n = n * base + c;
			term++;
		}
		DoSpaces();
		return sgn * n;
	}
	
	bool   IsDouble() const                   { return IsInt(); }
	bool   IsDouble2() const {
		const char *s = term;
		if(*s == '+' || *s == '-')
			s++;
		while(IsDigit(*s))
			s++;
		if(*s == '.')
			s++;
		while(IsDigit(*s))
			s++;
		if(*s == 'e' || *s == 'E') {
			s++;
			if(*s == '+' || *s == '-')
				s++;
			while(IsDigit(*s))
				s++;
		}
		return s > term;
	}
	
	double ReadDouble() {
		String s;
		if(Char('+'))
			s = "+";
		else if(Char('-'))
			s = "-";
		
		while(IsDigit(*term))
			s.Cat(GetChar());
		if(Char('.'))
			s.Cat('.');
		while(IsDigit(*term))
			s.Cat(GetChar());
		if(IsChar('e') || IsChar('E')) {
			s.Cat(GetChar());
			if(Char('+') || Char('-'))
				;
			while(IsDigit(*term))
				s.Cat(GetChar());
		}
		DoSpaces();
		return ::atof(~s);
	}
	
	double ReadDoubleNoE() {
		String s;
		if(Char('+'))
			s = "+";
		else if(Char('-'))
			s = "-";
		
		while(IsDigit(*term))
			s.Cat(GetChar());
		if(Char('.'))
			s.Cat('.');
		while(IsDigit(*term))
			s.Cat(GetChar());
		DoSpaces();
		return ::atof(~s);
	}
	
	bool   IsString() const                   { return IsChar('\"'); };
	
	String ReadOneString(bool chkend = true) {
		String s;
		if(!Char('\"')) return s;
		while(*term && *term != '\"') {
			int c = GetChar();
			if(c == '\\' && uescape) {
				c = GetChar();
				switch(c) {
				case 'n':  c = '\n'; break;
				case 't':  c = '\t'; break;
				case 'r':  c = '\r'; break;
				case 'b':  c = '\b'; break;
				case 'f':  c = '\f'; break;
				case 'v':  c = '\v'; break;
				case 'a':  c = '\a'; break;
				case '\\': c = '\\'; break;
				case '?':  c = '\?'; break;
				case '\'': c = '\''; break;
				case '\"': c = '\"'; break;
				case 'x':
				case 'X': {
					dword h;
					if(!ReadHex(h, 2))
						ThrowError("Hexadecimal digit expected");
					c = h;
					break;
				}
				default:
					if(c >= '0' && c <= '7') {
						c -= '0';
						if(*term >= '0' && *term <= '7') {
							c = c * 8 + *term - '0';
							term++;
							if(*term >= '0' && *term <= '7') {
								int i = c * 8 + *term - '0';
								if(i <= 255) {
									c = i;
									term++;
								}
							}
						}
						DoSpaces();
					}
					break;
				}
			}
			s.Cat(c);
		}
		if(chkend && !Char('\"'))
			ThrowError("End of string literal expected");
		return s;
	}
	
	String ReadString(bool chkend = true) {
		String result;
		while(IsString())
			result.Cat(ReadOneString(chkend));
		return result;
	}
	
	String ReadOneString(int delim, bool chkend = true) {
		String s;
		if(!Char(delim)) return s;
		while(*term && *term != delim) {
			int c = GetChar();
			if(c == '\\' && uescape) {
				c = GetChar();
				switch(c) {
				case 'n':  c = '\n'; break;
				case 't':  c = '\t'; break;
				case 'r':  c = '\r'; break;
				case 'b':  c = '\b'; break;
				case 'f':  c = '\f'; break;
				case 'v':  c = '\v'; break;
				case 'a':  c = '\a'; break;
				case '\\': c = '\\'; break;
				case '?':  c = '\?'; break;
				case '\'': c = '\''; break;
				case '\"': c = '\"'; break;
				case 'x':
				case 'X': {
					dword h;
					if(!ReadHex(h, 2))
						ThrowError("Hexadecimal digit expected");
					c = h;
					break;
				}
				default:
					if(c >= '0' && c <= '7') {
						c -= '0';
						if(*term >= '0' && *term <= '7') {
							c = c * 8 + *term - '0';
							term++;
							if(*term >= '0' && *term <= '7') {
								int i = c * 8 + *term - '0';
								if(i <= 255) {
									c = i;
									term++;
								}
							}
						}
						DoSpaces();
					}
					break;
				}
			}
			s.Cat(c);
		}
		if(chkend && !Char(delim))
			ThrowError("End of string literal expected");
		return s;
	}
	
	String ReadString(int delim, bool chkend = true) {
		String result;
		while(IsChar(delim))
			result.Cat(ReadOneString(delim, chkend));
		return result;
	}

	void   Skip() {
		while(*term && *term != ',' && *term != ';' && *term != ')')
			GetChar();
	}

	void   SkipTerm()                         { Skip(); }

	struct Pos {
		const char *ptr;
		const char *wspc;
		const char *lineptr;
		int         line;
		String      fn;
		
		int GetColumn(int tabsize = 4) const {
			int col = 0;
			const char *s = lineptr;
			while(s < ptr) {
				if(*s == '\t')
					col = (col / tabsize + 1) * tabsize;
				else
					col++;
				s++;
			}
			return col;
		}

		Pos(const char *ptr = NULL, int line = 1, String fn = Null) : ptr(ptr), line(line), fn(fn) {}
	};

	const char *GetPtr() const                { return (const char *)term; }
	const char *GetSpacePtr() const           { return (const char *)wspc; }

	Pos         GetPos() const {
		Pos p;
		p.ptr = term;
		p.wspc = wspc;
		p.lineptr = lineptr;
		p.line = line;
		p.fn = fn;
		return p;
	}
	
	void        SetPos(const Pos& pos) {
		term = pos.ptr;
		wspc = pos.wspc;
		lineptr = pos.lineptr;
		line = pos.line;
		fn = pos.fn;
	}

	bool   IsEof() const                      { return *term == '\0'; }
	operator bool() const                     { return *term; }

	int    GetLine() const                    { return line; }
	int    GetColumn(int tabsize = 4) const {
		int col = 0;
		const char *s = lineptr;
		while(s < term) {
			if(*s == '\t')
				col = (col / tabsize + 1) * tabsize;
			else
				col++;
			s++;
		}
		return col;
	}
	String GetFileName() const                { return fn; }

	static String LineInfoComment(const String& filename, int line = 1, int column = 1) {
		return String().Cat() << filename << "(" << line << "," << column << ")";
	}
	
	String GetLineInfoComment(int tabsize = 4) const {
		return LineInfoComment(fn, line, GetColumn(tabsize) + 1);
	}
	
	enum { LINEINFO_ESC = '\2' };
	
	void   Set(const char *ptr, const char *fn, int line = 1) {
		term = ptr;
		this->fn = fn;
		this->line = line;
		Spaces0();
	}
	
	void   Set(const char *ptr) {
		Set(ptr, "");
	}

	CParser& SkipSpaces(bool b = true)        { skipspaces = b; return *this; }
	CParser& NoSkipSpaces()                   { skipspaces = false; return *this; }
	CParser& UnicodeEscape(bool b = true)     { uescape = b; return *this; }
	CParser& SkipComments(bool b = true) {
		skipcomments = b;
		return *this;
	}
	CParser& NoSkipComments()                 { return SkipComments(false); }
	CParser& NestComments(bool b = true) {
		nestcomments = b;
		return *this;
	}
	CParser& NoNestComments()                 { return NestComments(false); }

	CParser(const char *ptr) {
		Set(ptr, "");
	}
	
	CParser(const char *ptr, const char *fn, int line = 1) {
		Set(ptr, fn, line);
	}
	
	CParser() {
		term = "";
		wspc = "";
		lineptr = "";
		line = 1;
		skipspaces = true;
		skipcomments = true;
		nestcomments = true;
		uescape = true;
	}
};

inline bool CParser::Char(char c)
{
	if(IsChar(c)) {
		term++;
		DoSpaces();
		return true;
	}
	return false;
}

inline bool CParser::Char2(char c1, char c2)
{
	if(IsChar2(c1, c2)) {
		term += 2;
		DoSpaces();
		return true;
	}
	return false;
}

inline bool CParser::Char3(char c1, char c2, char c3)
{
	if(IsChar3(c1, c2, c3)) {
		term += 3;
		DoSpaces();
		return true;
	}
	return false;
}

enum {
	ASCSTRING_SMART     = 0x01,
	ASCSTRING_OCTALHI   = 0x02,
	ASCSTRING_JSON      = 0x04,
};

String AsCString(const char *s, const char *end, int linemax = INT_MAX, const char *linepfx = NULL,
                 dword flags = 0);
String AsCString(const char *s, int linemax = INT_MAX, const char *linepfx = NULL,
                 dword flags = 0);
String AsCString(const String& s, int linemax = INT_MAX, const char *linepfx = NULL,
                 dword flags = 0);

#endif