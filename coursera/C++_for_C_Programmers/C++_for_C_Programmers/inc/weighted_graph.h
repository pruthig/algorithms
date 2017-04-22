#ifndef _WEIGHTED_GRAPH_H
#define _WEIGHTED_GRAPH_H

#include<vector>
#include<fstream>
#include<list>
#include<utility>
#include<string>
#include<sstream>
#include<iostream>

using namespace std;


class WeightedGraph{
protected:
	vector< list<pair<int, int> > > graph;
	vector<bool> visited;
	int number_of_vertices;
	int number_of_edges;
public:
	WeightedGraph();
	// Constructor for WeightedGraph which will read the file which contains the edges with weights.
	WeightedGraph(char *file_path);
	// Destructor
	~WeightedGraph();
	// Check presence of node in graph
	bool is_node_present(int v1);
	// WeightedGraph is vector of list, resize it if needed
	void add_node(int v1);
	// Remove a node from graph
	void remove_node(int v1);
	vector<bool>& get_visited_nodes();
	void remove_edge(int v1, int v2);
	// Only need to check presence of edge in list at v1
	bool is_edge_present(int v1, int v2);
	// Add an edge to graph
	void add_edge(int v1, int v2, int w);
	// Returns the number of edges, calculated in ctor.
	int get_num_of_edges();
	// Returns the number of vertices, calculated in ctor.
	int get_num_of_vertices();
	// Returns the neighbors in the form of list
	list<pair<int, int>> get_neighbors(int vertex);
	// Method to check if all vertices are visited.
	bool are_all_visited();
};

#endif