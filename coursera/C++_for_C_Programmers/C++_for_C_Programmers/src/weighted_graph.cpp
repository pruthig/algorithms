// Generic class that will hold implementation of graph data structure which can be used both for directed
// and undirected graph

#include "weighted_graph.h"

// Default constructor
WeightedGraph::WeightedGraph(){


}

// Constructor for WeightedGraph which will read the file which contains the edges with weights.
WeightedGraph::WeightedGraph(char *file_path) {

	int counter = 0;
	std::ifstream infile(file_path);

	std::string line;
	int vertex1 = 0, vertex2 = 0, edge_weight = 0;
	while (getline(infile, line))
	{
		std::istringstream lineStream(line);
		if (counter == 0) {
			counter = 1;

			lineStream >> number_of_vertices;
			// Initialize graph vector
			graph.resize(number_of_vertices);
			// Initialize visited vector
			visited.resize(number_of_vertices);

			std::fill(visited.begin(), visited.end(), false);
			continue;
		}

		lineStream >> vertex1 >> vertex2 >> edge_weight;

		number_of_edges++;
		add_edge(vertex1, vertex2, edge_weight);


	}

	// Correct the number of edges as graph is undirected
	number_of_edges /= 2;
}

// Destructor
WeightedGraph::~WeightedGraph() {
	// Delete array to mark nodes
	visited.clear();
	// delete all nodes
	graph.clear();
}

// Check presence of node in graph
bool WeightedGraph::is_node_present(int v1) {
	if (graph.at(v1).size() > 0)
		return true;
	else
		return false;
}

// WeightedGraph is vector of list, resize it if needed
void WeightedGraph::add_node(int v1) {
	if (v1 >= graph.size()) {
		graph.resize(v1 * 2);
	}
}

// Remove a node from graph
void WeightedGraph::remove_node(int v1) {
	graph.at(v1).clear();
	// remove it from its neighboring vertex list;
	for (auto& list_t : graph) {
		for (auto &temp_pair : list_t) {
			if (temp_pair.first == v1) {
				list_t.remove(std::make_pair(v1, temp_pair.second));
			}
		}
	}
}

vector<bool>& WeightedGraph::get_visited_nodes(){
	return visited;
}

void WeightedGraph::remove_edge(int v1, int v2) {
	list< pair<int, int> >& temp_list1 = graph.at(v1);
	for (auto &temp_pair : temp_list1) {
		if (temp_pair.first == v2) {
			temp_list1.remove(std::make_pair(v2, temp_pair.second));
			break;
		}
	}
	// do the same for v2 as well
	list< pair<int, int> >& temp_list2 = graph.at(v2);
	for (auto &temp_pair : temp_list2) {
		if (temp_pair.first == v1) {
			temp_list2.remove(std::make_pair(v1, temp_pair.second));
			break;
		}
	}
}

// Only need to check presence of edge in list at v1
bool WeightedGraph::is_edge_present(int v1, int v2) {
	list< pair<int, int> >& temp_list = graph.at(v1);
	for (auto &temp_pair : temp_list) {
		if (temp_pair.first == v2) {
			return true;
		}
	}
	return false;
}
// Add an edge to graph
void WeightedGraph::add_edge(int v1, int v2, int w) {
	if (!is_edge_present(v1, v2)) {
		graph.at(v1).push_back(std::make_pair(v2, w));
	}

	if (!is_edge_present(v2, v1)) {
		graph.at(v2).push_back(std::make_pair(v1, w));
	}
}

// Returns the number of edges, calculated in ctor.
int WeightedGraph::get_num_of_edges() {
	return number_of_edges;
}

// Returns the number of vertices, calculated in ctor.
int WeightedGraph::get_num_of_vertices(){
	return number_of_vertices;
}

// Returns the neighbors in the form of list
list<pair<int, int>> WeightedGraph::get_neighbors(int vertex) {
	return graph.at(vertex);
}

// Method to check if all vertices are visited.
bool WeightedGraph::are_all_visited() {
	for (auto a : visited) {
		if (a == false)
			return false;
	}
	return true;
}


