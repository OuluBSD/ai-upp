#pragma once
#ifndef _Core_Xmlize_h_
#define _Core_Xmlize_h_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Core.h"

class XmlIO;

template <class T>
void XmlAttrLoad(T& var, const String& text)
{
	var.XmlAttrLoad(text);
}

template <class T>
String XmlAttrStore(const T& var)
{
	return var.XmlAttrStore();
}

class XmlIO {
	XmlNode& node;
	bool     loading;
	Value    userdata;

public:
	bool IsLoading() const            { return loading; }
	bool IsStoring() const            { return !loading; }

	XmlNode& Node()                   { return node; }
	const XmlNode& Node() const       { return node; }

	XmlNode *operator->()             { return &node; }

	String GetAttr(const char *id)                    { return node.Attr(id); }
	void   SetAttr(const char *id, const String& val) { node.SetAttr(id, val); }

	template <class T> XmlIO operator()(const char *tag, T& var);
	template <class T> XmlIO List(const char *tag, const char *itemtag, T& var);
	template <class T, class X> XmlIO Var(const char *tag, T& var, X var_xmlize);
	template <class T, class X> XmlIO Array(const char *tag, T& var, X item_xmlize, const char *itemtag = "item");

	template <class T, class D> XmlIO operator()(const char *tag, T& var, const D& def);
	template <class T, class D> XmlIO List(const char *tag, const char *itemtag, T& var, const D& def);

	template <class T> XmlIO Attr(const char *id, T& var) {
		if(IsLoading())
			XmlAttrLoad(var, node.Attr(id));
		else
			node.SetAttr(id, XmlAttrStore(var));
		return *this;
	}

	template <class T, class D> XmlIO Attr(const char *id, T& var, const D& def) {
		if(IsLoading())
		    if(IsNull(node.Attr(id)))
		        var = def;
		    else
		        XmlAttrLoad(var, node.Attr(id));
		else
			node.SetAttr(id, IsNull(var) ? String() : XmlAttrStore(var));
		return *this;
	}

	XmlIO& operator()(const char *tag, bool& var) {
		if(IsLoading())
			var = node[tag].GetText() == "1";
		else
			node.Add(tag).SetText(var ? "1" : "0");
		return *this;
	}

	XmlIO& operator()(const char *tag, int& var) {
		if(IsLoading())
			var = ScanInt(node[tag].GetText());
		else
			node.Add(tag).SetText(AsString(var));
		return *this;
	}

	XmlIO& operator()(const char *tag, double& var) {
		if(IsLoading())
			var = ScanDouble(node[tag].GetText());
		else
			node.Add(tag).SetText(AsString(var));
		return *this;
	}

	XmlIO& operator()(const char *tag, String& var) {
		if(IsLoading())
			var = node[tag].GetText();
		else
			node.Add(tag).SetText(var);
		return *this;
	}

	XmlIO& operator()(const char *tag, WString& var) {
		if(IsLoading())
			var = ToUnicode(node[tag].GetText(), CHARSET_UTF8);
		else
			node.Add(tag).SetText(FromUnicode(var, CHARSET_UTF8));
		return *this;
	}

	template <class T>
	XmlIO& operator()(const char *tag, Vector<T>& var) {
		if(IsLoading()) {
			var.Clear();
			XmlNode n = node[tag];
			for(int i = 0; i < n.GetCount(); i++)
				var.Add() << n[i];
		}
		else {
			XmlNode n = node.Add(tag);
			for(int i = 0; i < var.GetCount(); i++)
				n.Add("item") << var[i];
		}
		return *this;
	}

	template <class T>
	XmlIO& operator()(const char *tag, Array<T>& var) {
		if(IsLoading()) {
			var.Clear();
			XmlNode n = node[tag];
			for(int i = 0; i < n.GetCount(); i++)
				var.Add() << n[i];
		}
		else {
			XmlNode n = node.Add(tag);
			for(int i = 0; i < var.GetCount(); i++)
				n.Add("item") << var[i];
		}
		return *this;
	}

	template <class K, class V>
	XmlIO& operator()(const char *tag, VectorMap<K, V>& var) {
		if(IsLoading()) {
			var.Clear();
			XmlNode n = node[tag];
			for(int i = 0; i < n.GetCount(); i += 2) {
				K key;
				V value;
				key << n[i];
				value << n[i + 1];
				var.Add(key, value);
			}
		}
		else {
			XmlNode n = node.Add(tag);
			for(int i = 0; i < var.GetCount(); i++) {
				n.Add("key") << var.GetKey(i);
				n.Add("value") << var[i];
			}
		}
		return *this;
	}

	template <class K, class V>
	XmlIO& operator()(const char *tag, ArrayMap<K, V>& var) {
		if(IsLoading()) {
			var.Clear();
			XmlNode n = node[tag];
			for(int i = 0; i < n.GetCount(); i += 2) {
				K key;
				V value;
				key << n[i];
				value << n[i + 1];
				var.Add(key, value);
			}
		}
		else {
			XmlNode n = node.Add(tag);
			for(int i = 0; i < var.GetCount(); i++) {
				n.Add("key") << var.GetKey(i);
				n.Add("value") << var[i];
			}
		}
		return *this;
	}

	XmlIO(XmlNode& node, bool loading = true) : node(node), loading(loading) {}
	XmlIO(XmlNode& node, const char *tag, bool loading = true) : node(node.Add(tag)), loading(loading) {}
};

template <class T>
XmlIO XmlIO::operator()(const char *tag, T& var) {
	XmlIO n(*this, tag);
	if(IsLoading() && n.Node().GetCount() == 0 && n.Node().GetAttrCount() == 0)
		return *this;
	Xmlize(n, var);
	return *this;
}

template <class T, class X>
XmlIO XmlIO::Var(const char *tag, T& var, X item_xmlize)
{
	XmlIO n(*this, tag);
	if(IsLoading() && n.Node().GetCount() == 0 && n.Node().GetAttrCount() == 0)
		return *this;
	item_xmlize(n, var);
	return *this;
}

template <class T, class X>
XmlIO XmlIO::Array(const char *tag, T& var, X item_xmlize, const char *itemtag)
{
	XmlIO n(*this, tag);
	if(IsLoading() && n.Node().GetCount() == 0 && n.Node().GetAttrCount() == 0)
		return *this;
	XmlizeContainer(n, itemtag, var, item_xmlize);
	return *this;
}

template <class T>
XmlIO XmlIO::List(const char *tag, const char *itemtag, T& var) {
	return Array(tag, var, [](XmlIO& io, ValueTypeOf<T>& data) { Xmlize(io, data); }, itemtag);
}

template <class T, class D>
XmlIO XmlIO::operator()(const char *tag, T& var, const D& def)
{
	XmlIO n(*this, tag);
	if(IsLoading() && n.Node().GetCount() == 0 && n.Node().GetAttrCount() == 0)
		var = def;
	else
		Xmlize(n, var);
	return *this;
}

template <class T, class D>
XmlIO XmlIO::List(const char *tag, const char *itemtag, T& var, const D& def)
{
	XmlIO n(*this, tag);
	if(IsLoading() && n.Node().GetCount() == 0 && n.Node().GetAttrCount() == 0)
		var = def;
	else
		Xmlize(n, itemtag, var);
	return *this;
}

template <class T, class X>
void XmlizeContainer(XmlIO& xml, const char *tag, T& data, X item_xmlize)
{
	if(xml.IsStoring())
		for(int i = 0; i < data.GetCount(); i++) {
			XmlIO io = xml.Add(tag);
			item_xmlize(io, data[i]);
		}
	else {
		data.Clear();
		for(int i = 0; i < xml->GetCount(); i++)
			if(xml->Node(i).IsTag(tag)) {
				XmlIO io = xml.At(i);
				item_xmlize(io, data.Add());
			}
	}
}

template<class T>
void XmlizeContainer(XmlIO& xml, const char *tag, T& data)
{
	XmlizeContainer(xml, tag, data, [](XmlIO& xml, ValueTypeOf<T>& data) { Xmlize(xml, data); });
}

template<class K, class V, class T>
void XmlizeMap(XmlIO& xml, const char *keytag, const char *valuetag, T& data)
{
	if(xml.IsStoring()) {
		for(int i = 0; i < data.GetCount(); i++)
			if(!data.IsUnlinked(i)) {
				XmlIO k = xml.Add(keytag);
				XmlizeStore(k, data.GetKey(i));
				XmlIO v = xml.Add(valuetag);
				XmlizeStore(v, data[i]);
			}
	}
	else {
		data.Clear();
		int i = 0;
		while(i < xml->GetCount() - 1 && xml->Node(i).IsTag(keytag) && xml->Node(i + 1).IsTag(valuetag)) {
			K key;
			XmlIO k = xml.At(i++);
			Xmlize(k, key);
			XmlIO v = xml.At(i++);
			Xmlize(v, data.Add(key));
		}
	}
}

template<class K, class V, class T>
void XmlizeSortedMap(XmlIO& xml, const char *keytag, const char *valuetag, T& data)
{
	if(xml.IsStoring()) {
		for(int i = 0; i < data.GetCount(); i++) {
			XmlIO k = xml.Add(keytag);
			XmlizeStore(k, data.GetKey(i));
			XmlIO v = xml.Add(valuetag);
			XmlizeStore(v, data[i]);
		}
	}
	else {
		data.Clear();
		int i = 0;
		while(i < xml->GetCount() - 1 && xml->Node(i).IsTag(keytag) && xml->Node(i + 1).IsTag(valuetag)) {
			K key;
			XmlIO k = xml.At(i++);
			Xmlize(k, key);
			XmlIO v = xml.At(i++);
			Xmlize(v, data.Add(key));
		}
	}
}

template<class K, class T>
void XmlizeIndex(XmlIO& xml, const char *keytag, T& data)
{
	if(xml.IsStoring()) {
		for(int i = 0; i < data.GetCount(); i++)
			if(!data.IsUnlinked(i)) {
				XmlIO io = xml.Add(keytag);
				XmlizeStore(io, data[i]);
			}
	}
	else {
		data.Clear();
		int i = 0;
		while(i < xml->GetCount() && xml->Node(i).IsTag(keytag)) {
			K k;
			XmlIO io = xml.At(i++);
			Xmlize(io, k);
			data.Add(k);
		}
	}
}

// Utility functions
String XmlizeFormat(const String& xml);
String XmlizeEscape(const String& text);
String XmlizeUnescape(const String& text);

// Xmlize functions for common types
void Xmlize(XmlIO& xml, bool& b);
void Xmlize(XmlIO& xml, int& i);
void Xmlize(XmlIO& xml, double& d);
void Xmlize(XmlIO& xml, String& s);
void Xmlize(XmlIO& xml, WString& s);

template <class T>
void Xmlize(XmlIO& xml, Vector<T>& v) {
	XmlizeContainer(xml, "item", v);
}

template <class T>
void Xmlize(XmlIO& xml, Array<T>& a) {
	XmlizeContainer(xml, "item", a);
}

template <class K, class V>
void Xmlize(XmlIO& xml, VectorMap<K, V>& m) {
	XmlizeMap<K, V>(xml, "key", "value", m);
}

template <class K, class V>
void Xmlize(XmlIO& xml, ArrayMap<K, V>& m) {
	XmlizeMap<K, V>(xml, "key", "value", m);
}

template <class T>
void Xmlize(XmlIO& xml, Index<T>& i) {
	XmlizeIndex<T>(xml, "item", i);
}

// XmlizeStore functions for common types
void XmlizeStore(XmlIO& xml, bool b);
void XmlizeStore(XmlIO& xml, int i);
void XmlizeStore(XmlIO& xml, double d);
void XmlizeStore(XmlIO& xml, const String& s);
void XmlizeStore(XmlIO& xml, const WString& s);

template <class T>
void XmlizeStore(XmlIO& xml, const Vector<T>& v) {
	XmlizeContainer(xml, "item", const_cast<Vector<T>&>(v));
}

template <class T>
void XmlizeStore(XmlIO& xml, const Array<T>& a) {
	XmlizeContainer(xml, "item", const_cast<Array<T>&>(a));
}

template <class K, class V>
void XmlizeStore(XmlIO& xml, const VectorMap<K, V>& m) {
	XmlizeMap<K, V>(xml, "key", "value", const_cast<VectorMap<K, V>&>(m));
}

template <class K, class V>
void XmlizeStore(XmlIO& xml, const ArrayMap<K, V>& m) {
	XmlizeMap<K, V>(xml, "key", "value", const_cast<ArrayMap<K, V>&>(m));
}

template <class T>
void XmlizeStore(XmlIO& xml, const Index<T>& i) {
	XmlizeIndex<T>(xml, "item", const_cast<Index<T>&>(i));
}

#endif