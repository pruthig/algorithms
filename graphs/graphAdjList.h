//Adjacency list representation of graph..
//This program is the base program and exposes API to other programs

#include<vector>

using std::vector;

class Graph {

	//A vector of vector...graph[i] denotes the vector of edges that are connected to graph[i]
	vector< vector<int> > graphVector;
public :
	int degree(int i);
	void addVertex(int v);
	void addEdge(int v, int w);
	bool deleteEdge(int v, int w);
	void dfs();
	vector<int>::iterator searchEdge(int v, int w);
};



