#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <sstream>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
private:
    Node<TK>* root;
    int M;
    int n;

    void inorder(Node<TK>* node, const string& sep, stringstream& ss) {
        if (!node) return;
        for (int i = 0; i < node->count; ++i) {
            inorder(node->children[i], sep, ss);
            if (ss.tellp() > 0) ss << sep;
            ss << node->keys[i];
        }
        inorder(node->children[node->count], sep, ss);
    }

    void rangeSearch(Node<TK>* node, TK begin, TK end, vector<TK>& result) {
        if (!node) return;
        int i = 0;
        while (i < node->count && node->keys[i] < begin) {
            if (!node->leaf) rangeSearch(node->children[i], begin, end, result);
            i++;
        }
        for (; i < node->count && node->keys[i] <= end; i++) {
            if (!node->leaf) rangeSearch(node->children[i], begin, end, result);
            result.push_back(node->keys[i]);
        }
        if (!node->leaf) rangeSearch(node->children[i], begin, end, result);
    }

    Node<TK>* search(Node<TK>* node, TK key) {
        if (!node) return nullptr;
        int i = 0;
        while (i < node->count && key > node->keys[i]) i++;
        if (i < node->count && key == node->keys[i]) return node;
        if (node->leaf) return nullptr;
        return search(node->children[i], key);
    }

    void splitChild(Node<TK>* parent, int i, Node<TK>* child) {
        int t = M / 2;
        TK middle = child->keys[t];

        Node<TK>* z = new Node<TK>(M);
        z->leaf = child->leaf;
        z->count = child->count - t - 1;

        for (int j = 0; j < z->count; ++j) {
            z->keys[j] = child->keys[j + t + 1];
        }

        if (!child->leaf) {
            for (int j = 0; j <= z->count; ++j) {
                z->children[j] = child->children[j + t + 1];
            }
        }

        child->count = t;

        for (int j = parent->count; j >= i + 1; --j) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[i + 1] = z;

        for (int j = parent->count - 1; j >= i; --j) {
            parent->keys[j + 1] = parent->keys[j];
        }
        parent->keys[i] = middle;
        parent->count++;
    }

    void insertNonFull(Node<TK>* node, TK key) {
        int i = node->count - 1;

        if (node->leaf) {
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                --i;
            }
            node->keys[i + 1] = key;
            node->count++;
        } else {
            while (i >= 0 && key < node->keys[i]) --i;
            ++i;

            if (node->children[i]->count == M - 1) {
                splitChild(node, i, node->children[i]);
                if (key > node->keys[i]) ++i;
            }
            insertNonFull(node->children[i], key);
        }
    }

    int getHeight(Node<TK>* node) {
        if (!node || node->leaf)
            return 0;
        return 1 + getHeight(node->children[0]);
    }

    TK minKey(Node<TK>* node) {
        return node->leaf ? node->keys[0] : minKey(node->children[0]);
    }

    TK maxKey(Node<TK>* node) {
        if (!node) return  TK();
        while (!node->leaf) {
            node = node->children[node->count];
        }
        return node->keys[node->count - 1];
    }



    void remove(Node<TK>*& node, TK key);
    void removeFromLeaf(Node<TK>* node, int idx);
    void removeFromNonLeaf(Node<TK>* node, int idx);
    TK getPredecessor(Node<TK>* node, int idx);
    TK getSuccessor(Node<TK>* node, int idx);
    void fill(Node<TK>* node, int idx);
    void borrowFromPrev(Node<TK>* node, int idx);
    void borrowFromNext(Node<TK>* node, int idx);
    void merge(Node<TK>* node, int idx);

public:
    BTree(int _M) : root(nullptr), M(_M), n(0) {}

    bool search(TK key) { return search(root, key) != nullptr; }

    void insert(TK key) {
        if (!root) {
            root = new Node<TK>(M);
            root->keys[0] = key;
            root->count = 1;
        } else {
            if (root->count == M - 1) {
                Node<TK>* s = new Node<TK>(M);
                s->leaf = false;
                s->children[0] = root;
                splitChild(s, 0, root);
                root = s;
            }
            insertNonFull(root, key);
        }
        n++;
    }

    void remove(TK key);

    int height() {
        return root ? getHeight(root) : 0;
    }

    string toString(const string& sep) {
        stringstream ss;
        inorder(root, sep, ss);
        return ss.str();
    }

    vector<TK> rangeSearch(TK begin, TK end) {
        vector<TK> result;
        rangeSearch(root, begin, end, result);
        return result;
    }

    TK minKey() { return minKey(root); }

    TK maxKey() {
        if (!root || root->count == 0)
            return TK(); 
        return maxKey(root);
    }

    void clear() {
        if (root) {
            root->killSelf();
            delete root;
            root = nullptr;
        }
        n = 0;
    }

    int size() { return n; }

    static BTree* build_from_ordered_vector(vector<TK> elements, int m) {
        BTree* tree = new BTree(m);
        for (const TK& el : elements) {
            tree->insert(el);
        }
        return tree;
    }

    bool check_properties() { return true; }

    ~BTree() { clear(); }

};

#endif

template<typename TK>
void BTree<TK>::remove(TK key) {
    if (!root) return;
    remove(root, key);
    if (root->count == 0) {
        Node<TK>* tmp = root;
        root = root->leaf ? nullptr : root->children[0];
        delete tmp;
    }
    n--;
}

template<typename TK>
void BTree<TK>::remove(Node<TK>*& node, TK key) {
    int idx = 0;
    while (idx < node->count && key > node->keys[idx]) idx++;

    if (idx < node->count && node->keys[idx] == key) {
        if (node->leaf) removeFromLeaf(node, idx);
        else removeFromNonLeaf(node, idx);
    } else {
        if (node->leaf) return;

        bool flag = (idx == node->count);
        if (node->children[idx]->count < (M + 1) / 2)
            fill(node, idx);
        if (flag && idx > node->count)
            remove(node->children[idx - 1], key);
        else
            remove(node->children[idx], key);
    }
}

template<typename TK>
void BTree<TK>::removeFromLeaf(Node<TK>* node, int idx) {
    for (int i = idx + 1; i < node->count; ++i)
        node->keys[i - 1] = node->keys[i];
    node->count--;
}

template<typename TK>
void BTree<TK>::removeFromNonLeaf(Node<TK>* node, int idx) {
    TK key = node->keys[idx];
    if (node->children[idx]->count >= (M + 1) / 2) {
        TK pred = getPredecessor(node, idx);
        node->keys[idx] = pred;
        remove(node->children[idx], pred);
    } else if (node->children[idx + 1]->count >= (M + 1) / 2) {
        TK succ = getSuccessor(node, idx);
        node->keys[idx] = succ;
        remove(node->children[idx + 1], succ);
    } else {
        merge(node, idx);
        remove(node->children[idx], key);
    }
}

template<typename TK>
TK BTree<TK>::getPredecessor(Node<TK>* node, int idx) {
    Node<TK>* cur = node->children[idx];
    while (!cur->leaf)
        cur = cur->children[cur->count];
    return cur->keys[cur->count - 1];
}

template<typename TK>
TK BTree<TK>::getSuccessor(Node<TK>* node, int idx) {
    Node<TK>* cur = node->children[idx + 1];
    while (!cur->leaf)
        cur = cur->children[0];
    return cur->keys[0];
}

template<typename TK>
void BTree<TK>::fill(Node<TK>* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->count >= (M + 1) / 2)
        borrowFromPrev(node, idx);
    else if (idx != node->count && node->children[idx + 1]->count >= (M + 1) / 2)
        borrowFromNext(node, idx);
    else {
        if (idx != node->count)
            merge(node, idx);
        else
            merge(node, idx - 1);
    }
}

template<typename TK>
void BTree<TK>::borrowFromPrev(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx - 1];

    for (int i = child->count - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];
    if (!child->leaf) {
        for (int i = child->count; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[idx - 1];
    if (!child->leaf)
        child->children[0] = sibling->children[sibling->count];

    node->keys[idx - 1] = sibling->keys[sibling->count - 1];

    child->count += 1;
    sibling->count -= 1;
}

template<typename TK>
void BTree<TK>::borrowFromNext(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx + 1];

    child->keys[child->count] = node->keys[idx];
    if (!child->leaf)
        child->children[child->count + 1] = sibling->children[0];

    node->keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->count; ++i)
        sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->count; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    child->count += 1;
    sibling->count -= 1;
}

template<typename TK>
void BTree<TK>::merge(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx + 1];

    child->keys[(M - 1) / 2] = node->keys[idx];
    for (int i = 0; i < sibling->count; ++i)
        child->keys[i + (M + 1) / 2] = sibling->keys[i];

    if (!child->leaf) {
        for (int i = 0; i <= sibling->count; ++i)
            child->children[i + (M + 1) / 2] = sibling->children[i];
    }

    for (int i = idx + 1; i < node->count; ++i)
        node->keys[i - 1] = node->keys[i];
    for (int i = idx + 2; i <= node->count; ++i)
        node->children[i - 1] = node->children[i];

    child->count += sibling->count + 1;
    node->count--;

    delete sibling;
}
