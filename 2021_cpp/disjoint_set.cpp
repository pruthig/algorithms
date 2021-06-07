#include<iostream>

using std::cout;
using std::endl;

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
    DisjointSet(int v_) {
        setArr = nullptr;
        if(v_ <= 0) {
            size = 0;
            return;
        }
        size = v_;
        make_set(size);
    }
    void make_set(int v);
    int find_parent(int element);
    void union_set(int x, int y);
    int count_set();
};

// Make the set of v vertices
void DisjointSet::make_set(int v) {
    if(v <= 0)
        return;
    setArr = new SetNode[v];
    for(int i=0;i<v;++i) {
        SetNode node;
        node.data = i;
        node.parent = i;
        node.rank = 0;
        setArr[i] = node;
    }
}

// Find the node in set
int DisjointSet::find_parent(int element) {
    if(!setArr)
        return -1;
    int parent = setArr[element].parent;
    if(parent == element)
        return parent;
    else {
        int parent = find_parent(parent);
        // Path compression technique
        setArr[element].parent = parent;
        return parent;
     }
}

// Join the sets  of nodes x and y.
void DisjointSet::union_set(int x, int y) {
    if(!setArr)
        return;
    int parent_x = setArr[x].parent;
    int parent_y = setArr[y].parent;
    if(parent_x == parent_y)
        return;
        
    else {
        // Union by rank
        if(setArr[parent_x].rank >setArr[parent_y].rank)
            setArr[parent_y].parent = parent_x;
        else if(setArr[parent_y].rank >setArr[parent_x].rank)
            setArr[parent_x].parent = parent_y;
        else {
            setArr[parent_y].parent = parent_x;
            setArr[parent_x].rank++;
        }
    }
}

int DisjointSet::count_set() {
    int count = 0;
    if(setArr == nullptr)
        return count;
        
    for(int i=0;i<size;++i) {
        if(find_parent(setArr[i].data) == setArr[i].data)
            ++count;
    }
    return count;
}

/*
int main() {
    // Test the disjoint sets
    DisjointSet disjointSet(5);
    disjointSet.union_set(0, 4);
    disjointSet.union_set(1, 4);
    disjointSet.union_set(3, 4);
    int a = disjointSet.find_parent(2);
    int b = disjointSet.find_parent(1);
    cout<<"Parent of 2 and 1 are" <<a<<" and "<<b<<"respectively"<<endl;
    return 0;
}
*/