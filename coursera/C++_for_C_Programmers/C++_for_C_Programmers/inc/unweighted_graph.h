
#ifndef _UNWEIGHTED_GRAPH_H
#define _UNWEIGHTED_GRAPH_H

#include<vector>
#include<list>

using namespace std;


class UnweightedGraph {
protected:
	vector< list<int> > graph;
	vector<bool> visited;
public:
	// Create vector of nodes.
	UnweightedGraph(int num_vertices);
	UnweightedGraph();
	~UnweightedGraph();
	bool is_node_present(int v1);
	void add_node(int v1);
	void remove_node(int v1);
	vector<bool>& get_visited_nodes();
	void remove_edge(int v1, int v2);
	bool is_edge_present(int v1, int v2);
	void add_edge(int v1, int v2);
	int get_num_of_edges();
	int get_num_of_vertices();
	list<int> get_neighbors(int vertex);
	bool are_all_visited();

};

#endif