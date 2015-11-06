//Adjacency list representation of graph..
//This program is the base program and exposes API to other programs

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
    bool *marked;
    int *edgeTo;
    int *id;  //Array to store the component number of a given vertex
    int counter; //It stores the count for number of strongly connected components
    int vertexCount;
public :
	int degree(int i);
	void addVertex(int v);
    void init(int v);

	void dfs(int h);
    void dfsWithStack(int v);
    void cc();

    void bfs(int v);
    void bfsWithQueue(int v);
	void addEdge(int v, int w);
	bool deleteEdge(int v, int w);
	vector<int>::iterator searchEdge(int v, int w);
};



