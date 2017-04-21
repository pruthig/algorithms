// Generic class that will hold implementation of UnweightedGraph data structure which can be used both for directed
// and undirected UnweightedGraph

#include "unweighted_graph.h"

// Default constructor
UnweightedGraph::UnweightedGraph(){
	graph.resize(1);
}

// Create vector of nodes.
UnweightedGraph::UnweightedGraph(int num_vertices) {
	graph.resize(num_vertices);
	// Initialize visited vector
	visited.resize(num_vertices);
}

// Destructor
UnweightedGraph::~UnweightedGraph() {
	// Delete array to mark nodes
	visited.clear();
	// delete all nodes
	graph.clear();
}

// Check presence of node in UnweightedGraph
bool UnweightedGraph::is_node_present(int v1) {
	if (v1 > graph.size()-1)
		return false;

	if (graph.at(v1).size() > 0)
		return true;
	else
		return false;
}

// UnweightedGraph is vector of list, resize it if needed
void UnweightedGraph::add_node(int v1) {
	if (v1 >= graph.size()) {
		graph.resize(v1 * 2);
	}
}

// Remove a node from UnweightedGraph
void UnweightedGraph::remove_node(int v1) {
	// remove all edges
	graph.at(v1).clear();
	// remove it from its neighbour's edge list;
	for (auto& list_t : graph) {
		list_t.remove(v1);
	}
}

// return visited nodes
vector<bool>& UnweightedGraph::get_visited_nodes(){
	return visited;
}

// remove edge
void UnweightedGraph::remove_edge(int v1, int v2) {
	graph.at(v1).remove(v2);
	graph.at(v2).remove(v1);
}

// Check presence of edge in list at v1
bool UnweightedGraph::is_edge_present(int v1, int v2) {
	list<int>::iterator list_itr = find(graph.at(v1).begin(), graph.at(v1).end(), v2);
	if (list_itr != graph.at(v1).end())
		return true;
	else
		return false;
}

// Add an edge to UnweightedGraph
void UnweightedGraph::add_edge(int v1, int v2) {
	if (!is_node_present(v1))
		add_node(v1);

	if (!is_node_present(v2))
		add_node(v2);

	if (!is_edge_present(v1, v2)) {
		graph.at(v1).push_back(v2);
		graph.at(v2).push_back(v1);
	}
}

// Returns the number of edges, calculated in ctor.
int UnweightedGraph::get_num_of_edges() {
	int num = 0;
	for (auto& list : graph) {
		for (auto& edge : list)
			num++;
	}
	// graph is undirected
	return num/2;
}

// Returns the number of vertices, calculated in ctor.
int UnweightedGraph::get_num_of_vertices(){
	int num = 0;
	for (auto& list_t : graph) {
		if (list_t.size() > 0)
			num++;
	}
	return num;
}

// Returns the neighbors in the form of list
list<int> UnweightedGraph::get_neighbors(int vertex) {
	return graph.at(vertex);
}

// Method to check if all vertices are visited.
bool UnweightedGraph::are_all_visited() {
	for (auto a : visited) {
		if (a == false)
			return false;
	}
	return true;
}
