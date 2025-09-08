// CritBitIndex.h
// Header-only Patricia/Crit-bit index over fixed-width hash keys
#pragma once

#include <Core/Core.h>

// Generic, compressed binary trie (Patricia / crit-bit) keyed by fixed-width unsigned hash
// Uses MSB-first bit order (bit 0 is the highest bit).
// Container stores records T and an associated Hash key per record.

template <bool B, class T, class F>
struct __CrtbSelect { using Result = T; };
template <class T, class F>
struct __CrtbSelect<false, T, F> { using Result = F; };

template <class T, class GetHash, int Bits = 64>
class CritBitIndex {
public:
    using Hash   = typename __CrtbSelect<Bits == 32, dword, uint64>::Result;
    using Size   = int;
    using Handle = int;

    CritBitIndex() : root_(-1) {}

    void Clear() {
        nodes_.Clear();
        recs_.Clear();
        keys_.Clear();
        leaf_of_rec_.Clear();
        root_ = -1;
    }

    bool IsEmpty() const          { return recs_.IsEmpty(); }
    Size GetCount() const         { return recs_.GetCount(); }

    T* Find(Hash h)               { return const_cast<T*>(static_cast<const CritBitIndex*>(this)->Find(h)); }

    const T* Find(Hash h) const {
        int n = root_;
        while(n >= 0) {
            const Node& nd = nodes_[n];
            if(nd.idx_bit < 0)
                return keys_[nd.rec] == h ? &recs_[nd.rec] : nullptr;
            n = Bit(h, nd.idx_bit) ? nd.right : nd.left;
        }
        return nullptr;
    }

    // Upsert by key: default-construct if not present
    T* Put(Hash h) {
        if(T* p = Find(h))
            return p;
        int rec = recs_.GetCount();
        recs_.Add(T());
        keys_.Add(h);
        leaf_of_rec_.Add(-1);
        if(root_ < 0) {
            int leaf = NewLeaf(rec);
            leaf_of_rec_[rec] = leaf;
            root_ = leaf;
            return &recs_[rec];
        }
        InsertRecord(h, rec);
        return &recs_[rec];
    }

    T* Put(Hash h, const T& value) {
        T* p = Put(h);
        *p = value;
        return p;
    }

    T* Put(Hash h, T&& value) {
        T* p = Put(h);
        *p = pick(value);
        return p;
    }

    bool Remove(Hash h) {
        if(root_ < 0)
            return false;
        bool removed = false;
        int  removed_rec = -1;
        int  removed_leaf = -1;
        root_ = RemoveAt(root_, h, removed, removed_leaf, removed_rec);
        if(!removed)
            return false;

        // Maintain dense recs_/keys_ using swap-delete and fix leaf_of_rec_ mapping
        int last = recs_.GetCount() - 1;
        if(removed_rec != last) {
            // Move last into removed_rec slot
            recs_[removed_rec] = pick(recs_.Top());
            keys_[removed_rec] = keys_.Top();
            int moved_leaf = leaf_of_rec_.Top();
            if(moved_leaf >= 0) {
                nodes_[moved_leaf].rec = removed_rec;
                leaf_of_rec_[removed_rec] = moved_leaf;
            }
        }
        recs_.Drop();
        keys_.Drop();
        leaf_of_rec_.Drop();
        return true;
    }

    void Optimize() {
        if(root_ >= 0)
            root_ = OptimizeAt(root_);
    }

    void Merge(CritBitIndex& other, bool keepThis = true) {
        for(int i = 0; i < other.recs_.GetCount(); i++) {
            T& t = other.recs_[i];
            Hash h = other.keys_[i];
            // Prefer provided accessor if keys are not trusted externally
            // But here we keep 'h' from other to avoid recomputing
            T* dst = Find(h);
            if(!dst)
                Put(h, pick(t));
            else if(!keepThis)
                *dst = pick(t);
        }
        other.Clear();
    }

    void PutBulk(const Vector<Hash>& keys, Vector<T*>& out) {
        out.Clear();
        out.SetCount(keys.GetCount());
        recs_.Reserve(recs_.GetCount() + keys.GetCount());
        nodes_.Reserve(nodes_.GetCount() + keys.GetCount() * 2);
        leaf_of_rec_.Reserve(leaf_of_rec_.GetCount() + keys.GetCount());
        for(int i = 0; i < keys.GetCount(); i++)
            out[i] = Put(keys[i]);
    }

    template <class Fn>
    void ForEach(Fn&& fn) {
        for(int i = 0; i < recs_.GetCount(); i++)
            fn(recs_[i]);
    }

private:
    struct Node {
        int idx_bit; // -1 for leaf
        int left;
        int right;
        int rec;     // valid for leaf
    };

    Array<Node>  nodes_;
    Array<T>     recs_;
    Vector<Hash> keys_;
    Vector<int>  leaf_of_rec_; // rec_index -> leaf node index

    int          root_;
    GetHash      get_;

    // Bit utilities
    static bool Bit(Hash h, int bit) {
        int shift = (Bits - 1) - bit;
        return ((h >> shift) & 1) != 0;
    }

    static int Divergence(Hash a, Hash b) {
        Hash x = a ^ b;
        if(x == 0)
            return -1;
        int hi;
        if constexpr(Bits == 64)
            hi = SignificantBits64(x) - 1; // index from LSB
        else
            hi = SignificantBits((dword)x) - 1;
        return (Bits - 1) - hi;            // index from MSB
    }

    int NewLeaf(int rec_index) {
        int id = nodes_.GetCount();
        Node& n = nodes_.Add();
        n.idx_bit = -1;
        n.left = -1;
        n.right = -1;
        n.rec = rec_index;
        return id;
    }

    int NewInternal(int bit, int left, int right) {
        int id = nodes_.GetCount();
        Node& n = nodes_.Add();
        n.idx_bit = bit;
        n.left = left;
        n.right = right;
        n.rec = -1;
        return id;
    }

    void InsertRecord(Hash h, int rec_index) {
        // Two-phase insertion with local rotation on return to maintain order
        root_ = InsertAt(root_, h, rec_index);
    }

    int InsertAt(int n, Hash h, int rec_index) {
        if(n < 0) {
            int leaf = NewLeaf(rec_index);
            leaf_of_rec_[rec_index] = leaf;
            return leaf;
        }

        Node& nd = nodes_[n];
        if(nd.idx_bit < 0) {
            // Leaf: either duplicate or create internal at divergence bit
            Hash h0 = keys_[nd.rec];
            int d = Divergence(h, h0);
            if(d < 0) {
                // Duplicate: should not happen as caller checks, but be defensive
                return n;
            }
            int newleaf = NewLeaf(rec_index);
            leaf_of_rec_[rec_index] = newleaf;
            bool bnew = Bit(h, d);
            int left  = bnew ? n : newleaf;
            int right = bnew ? newleaf : n;
            return NewInternal(d, left, right);
        }

        // Internal: descend
        bool b = Bit(h, nd.idx_bit);
        int child = b ? nd.right : nd.left;
        int newchild = InsertAt(child, h, rec_index);

        if(newchild == child)
            return n; // no structural change

        // If new split is above current bit, rotate newchild above this node
        const Node& nc = nodes_[newchild];
        if(nc.idx_bit >= 0 && nc.idx_bit < nd.idx_bit) {
            // newchild is internal with a shallower bit. Attach this node where old child was
            Node& mm = nodes_[newchild];
            if(mm.left == child)
                mm.left = n;
            else if(mm.right == child)
                mm.right = n;
            // Keep original child link in 'nd' unchanged (it still points to 'child')
            return newchild;
        }

        // Normal attach
        if(b) nd.right = newchild; else nd.left = newchild;
        return n;
    }

    int RemoveAt(int n, Hash h, bool& removed, int& removed_leaf_node, int& removed_rec) {
        if(n < 0)
            return -1;
        Node& nd = nodes_[n];
        if(nd.idx_bit < 0) {
            if(keys_[nd.rec] == h) {
                removed = true;
                removed_rec = nd.rec;
                removed_leaf_node = n;
                return -1; // delete this leaf
            }
            return n;
        }
        bool b = Bit(h, nd.idx_bit);
        int child = b ? nd.right : nd.left;
        int newchild = RemoveAt(child, h, removed, removed_leaf_node, removed_rec);
        if(b) nd.right = newchild; else nd.left = newchild;

        // Collapse if possible
        int l = nd.left, r = nd.right;
        if(l < 0 && r < 0) return -1;
        if(l < 0) return r;
        if(r < 0) return l;
        return n;
    }

    int OptimizeAt(int n) {
        if(n < 0)
            return -1;
        Node& nd = nodes_[n];
        if(nd.idx_bit < 0)
            return n;
        nd.left = OptimizeAt(nd.left);
        nd.right = OptimizeAt(nd.right);
        if(nd.left < 0) return nd.right;
        if(nd.right < 0) return nd.left;
        return n;
    }
};

