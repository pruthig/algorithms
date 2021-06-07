#ifndef _DISJOINT_SETS
#define _DISJOINT_SETS

struct SetNode {
    int data;
    int rank;
    int parent;
};

class DisjointSet {
    SetNode *setArr;
    int size;
public:
    DisjointSet() {
        setArr = nullptr;
        size = 0;
    }
    void make_set(int v);
    int find_parent(int element);
    void union_set(int x, int y);
};

#endif