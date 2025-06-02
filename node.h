#ifndef NODE_H
#define NODE_H

using namespace std;

template <typename TK>
struct Node {
    // array de keys
    TK* keys;
    // array de punteros a hijos
    Node** children;
    // cantidad de keys
    int count;
    // indicador de nodo hoja
    bool leaf;

    Node() : keys(nullptr), children(nullptr), count(0), leaf(true) {}
    Node(int M) {
        keys = new TK[M - 1];
        children = new Node<TK>*[M];
        count = 0;
        leaf = true;
        for (int j = 0; j < M; ++j) {
            children[j] = nullptr;
        }
    }

    void killSelf() {
        for (int i = 0; i <= count; ++i) {
            if (children[i]) {
                children[i]->killSelf();
                delete children[i];
                children[i] = nullptr;
            }
        }
        delete[] keys;
        delete[] children;
    }

};

#endif
