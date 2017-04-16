// Declaration of graph data structure

#ifndef _GRAPH_H
#define _GRAPH_H

#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <utility>
#include <sstream>
#include <queue>
#include <algorithm>
#include <ctime>
#include <climits>
#include <cstdlib>

using namespace std;


// Main usable Graph class to implement the Prim's algorithm to find the minimum spanning tree and
// calculate its weight. The class contains other utility function which are not used in current program but 
// can be used to implement any other applications involving Graphs.
class Graph{
protected:
	// Main graph data struct
	vector< list< pair<int, int> > > graph;
	// bool vector to mark visited nodes
	vector<bool> visited;
	// Total weight of graph to be calculated for MST.
	int total_weight;
	// Number of vertices
	int number_of_vertices;
	// Number of edges
	int number_of_edges;

public:
	Graph();
	Graph(char *file_path);
	~Graph();
	int get_num_of_edges();
	int get_num_of_vertices();
	int get_total_weight();
	vector<bool>& get_visited_nodes();
	void add_edge(int v1, int v2, int w);
	void remove_edge(int v1, int v2);

	void add_node(int v1);
	void remove_node(int v1);
	list< pair<int, int> > get_neighbors(int vertex);

	bool is_edge_present(int v1, int v2);
	bool is_node_present(int v1);
	bool are_all_visited();
};


#endif