#pragma once
#ifndef _Core_BinUndoRedo_h_
#define _Core_BinUndoRedo_h_

#include <string>
#include <vector>
#include "Core.h"

String BinDiff(const String& base, const String& data);
String BinUndiff(const String& base, const String& bin_diff);

class BinUndoRedo {
	struct Entry : Moveable<Entry> {
		String data;
		Value  info;
	};
	Entry          commit;
	Vector<Entry>  undo;
	Vector<Entry>  redo;
	int            undosize = 0;

public:
	void   Reset(const String& current, const Value& info = Value());
	bool   Commit(const String& current, const Value& info, int limit = 4096*1024);
	bool   Commit(const String& current, int limit = 4096*1024) { return Commit(current, String(), limit); }
	bool   IsUndo() const                                       { return undo.GetCount(); }
	bool   IsRedo() const                                       { return redo.GetCount(); }
	int    GetUndoCount() const                                 { return undo.GetCount(); }
	int    GetRedoCount() const                                 { return redo.GetCount(); }
	bool   DropUndo();
	bool   DropRedo();
	Value  GetUndoInfo(int i) const                             { return undo[i].info; }
	Value  GetRedoInfo(int i) const                             { return redo[i].info; }
	Value  GetCommitInfo() const                                { return commit.info; }
	String Undo(const String& current);
	String Redo(const String& current);
	
	// Additional methods from uppsrc version
	void   Clear()                                              { Reset(String(), Value()); }
	bool   CanUndo() const                                      { return IsUndo(); }
	bool   CanRedo() const                                      { return IsRedo(); }
	Value  GetInfo() const                                      { return GetCommitInfo(); }
	int    GetCount() const                                     { return undo.GetCount() + redo.GetCount() + 1; }
	bool   IsEmpty() const                                      { return !commit.data.GetCount() && undo.IsEmpty() && redo.IsEmpty(); }
	
	// Serialization support
	void   Serialize(Stream& s) {
		s % commit.data % commit.info % undo % redo % undosize;
	}
	
	// Streaming operator
	template<typename Stream>
	friend void operator%(Stream& s, BinUndoRedo& bur) {
		bur.Serialize(s);
	}
	
	// String representation
	String ToString() const {
		return "BinUndoRedo(undo=" + AsString(undo.GetCount()) + 
		       ", redo=" + AsString(redo.GetCount()) + 
		       ", size=" + AsString(undosize) + ")";
	}
};

// Implementation of BinDiff and BinUndiff functions
inline String BinDiff(const String& base, const String& data) {
	// Simple binary diff implementation - for now just return the data
	// A more sophisticated implementation would compute the actual differences
	return data;
}

inline String BinUndiff(const String& base, const String& bin_diff) {
	// Simple binary undiff implementation - for now just return the diff
	// A more sophisticated implementation would apply the differences to the base
	return bin_diff;
}

// Implementation of BinUndoRedo methods
inline void BinUndoRedo::Reset(const String& current, const Value& info) {
	commit.data = current;
	commit.info = info;
	undo.Clear();
	redo.Clear();
	undosize = 0;
}

inline bool BinUndoRedo::Commit(const String& current, const Value& info, int limit) {
	if(current == commit.data)
		return false;
	
	Entry& e = undo.Add();
	e.data = BinDiff(commit.data, current);
	e.info = info;
	undosize += e.data.GetCount();
	
	while(undo.GetCount() > 1 && undosize > limit) {
		undosize -= undo[0].data.GetCount();
		undo.Remove(0);
	}
	
	redo.Clear();
	commit.data = current;
	commit.info = info;
	return true;
}

inline bool BinUndoRedo::DropUndo() {
	if(undo.IsEmpty())
		return false;
	undosize -= undo[0].data.GetCount();
	undo.Remove(0);
	return true;
}

inline bool BinUndoRedo::DropRedo() {
	if(redo.IsEmpty())
		return false;
	redo.Remove(0);
	return true;
}

inline String BinUndoRedo::Undo(const String& current) {
	if(undo.IsEmpty())
		return current;
	Entry e = undo.Pop();
	redo.Add().data = BinDiff(current, commit.data);
	redo.Top().info = commit.info;
	commit.data = current;
	commit.info = e.info;
	undosize -= e.data.GetCount();
	return BinUndiff(current, e.data);
}

inline String BinUndoRedo::Redo(const String& current) {
	if(redo.IsEmpty())
		return current;
	Entry e = redo.Pop();
	undo.Add().data = BinDiff(current, commit.data);
	undo.Top().info = commit.info;
	commit.data = current;
	commit.info = e.info;
	undosize += undo.Top().data.GetCount();
	return BinUndiff(current, e.data);
}

// Additional utility functions
inline void BinUndoRedo::Ids(Event<const String&> fn) {
	// Apply function to all IDs in the undo/redo stacks
	// This is a simplified implementation
	for(const auto& entry : undo) {
		if(entry.info.Is<String>()) {
			fn(entry.info);
		}
	}
	for(const auto& entry : redo) {
		if(entry.info.Is<String>()) {
			fn(entry.info);
		}
	}
	if(commit.info.Is<String>()) {
		fn(commit.info);
	}
}

#endif