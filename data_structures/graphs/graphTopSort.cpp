#include<vector>
#include<cstdlib>
#include<stack>
#include<queue>
#include<iostream>

using std::vector;
using std::cin;
using std::endl;
using std::cout;
using std::stack;
using std::queue;

class Graph {

    //A vector of vector...graph[i] denotes the vector of edges that are connected to graph[i]
    vector< vector<int> > graphVector;
    stack<int> stck, reverse;
    bool *marked;
    int vertexCount;
public :
    void addVertex(int v); 
    void init(int v); 
    void dfsWithStack(int v); 
    void topSort();
    void addEdge(int v, int w);
};

void Graph::init(int a){
    vertexCount = a;
    marked = new bool[a];
    for(int i = 0 ; i < a; ++i)
        marked[i] = false;
    addVertex(a);
}


void Graph::addEdge(int v, int w) {
  graphVector[v].push_back(w);
}

void Graph::addVertex(int v) {
    for(int i = 0; i < v; ++i) {
        vector<int> a;
        a.clear();
    	graphVector.push_back(a);
    }
}


void Graph::dfsWithStack(int vertex) {
    
    marked[vertex] = true;
    vector<int> vect = graphVector[vertex];
    for(int w : vect) {
        
        if(!marked[w]) {
            dfsWithStack(w);
        }

    }

    reverse.push(vertex);

}

void Graph::topSort() {
    for(int  i = 0; i < vertexCount; ++i) {
        if(!marked[i]) {
            dfsWithStack(i);
        }
    }
    
    while(!reverse.empty()){
        cout<<", "<<reverse.top()<<", ";
        reverse.pop();
    }
}


//To cover DFS we need to have double entries for edges: x,y and y,x
int main(){
	Graph gObject;
    gObject.init(10);

    gObject.addEdge(0, 1); 

    gObject.addEdge(1, 2); 
    gObject.addEdge(1, 6); 

    gObject.addEdge(2, 6); 
    gObject.addEdge(2, 7); 
    gObject.addEdge(2, 8); 

    gObject.addEdge(3,2); 
    gObject.addEdge(3, 8); 
    gObject.addEdge(3, 9); 

    gObject.addEdge(4, 3); 

    gObject.addEdge(5,0); 

    gObject.addEdge(6, 0); 
    gObject.addEdge(6, 5); 

    gObject.addEdge(7, 6); 
    gObject.addEdge(7, 8); 


    gObject.addEdge(9,4); 
    gObject.addEdge(9, 8); 
    gObject.topSort();
    cout<<endl;
	return 0;
}
