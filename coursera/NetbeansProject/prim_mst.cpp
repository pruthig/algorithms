
/*
* Prim's Minimum Spanning Tree algorithm.
*/

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
private:
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
	~Graph();
	int get_num_of_edges();
	int get_num_of_vertices();
	int get_total_weight();

	void add_edge(int v1, int v2, int w);
	void remove_edge(int v1, int v2);

	void add_node(int v1);
	void remove_node(int v1);
	list< pair<int, int> > get_neighbors(int vertex);

	bool is_edge_present(int v1, int v2);
	bool is_node_present(int v1);
	bool are_all_visited();
	void find_MST();

};

// Constructor for the MST algorithm. It will read the file which contains the edges with weights.
Graph::Graph() {

	int counter = 0;
	std::ifstream infile("MST.txt");

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
	// Counter to sum up the minimum weight of graph
	total_weight = 0;
	// Correct the number of edges as graph is undirected
	number_of_edges /= 2;
}

// Destructor
Graph::~Graph() {
	// Delete array to mark nodes
	visited.clear();
	// delete all nodes
	graph.clear();
}

// Check presence of node in graph
bool Graph::is_node_present(int v1) {
	if (graph.at(v1).size() > 0)
		return true;
	else
		return false;
}

// Graph is vector of list, resize it if needed
void Graph::add_node(int v1) {
	if (v1 >= graph.size()) {
		graph.resize(v1 * 2);
	}
}

// Remove a node from graph
void Graph::remove_node(int v1) {
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
void Graph::remove_edge(int v1, int v2) {
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
bool Graph::is_edge_present(int v1, int v2) {
	list< pair<int, int> >& temp_list = graph.at(v1);
	for (auto &temp_pair : temp_list) {
		if (temp_pair.first == v2) {
			return true;
		}
	}
	return false;
}
// Add an edge to graph
void Graph::add_edge(int v1, int v2, int w) {
	if (!is_edge_present(v1, v2)) {
		graph.at(v1).push_back(std::make_pair(v2, w));
	}

	if (!is_edge_present(v2, v1)) {
		graph.at(v2).push_back(std::make_pair(v1, w));
	}
}

// Returns the number of edges, calculated in ctor.
int Graph::get_num_of_edges() {
	return number_of_edges;
}

// Returns the number of vertices, calculated in ctor.
int Graph::get_num_of_vertices(){
	return number_of_vertices;
}

// Returns the neighbors in the form of list
list<pair<int, int>> Graph::get_neighbors(int vertex) {
	return graph.at(vertex);
}

// Method to check if all vertices are visited.
bool Graph::are_all_visited() {
	for (auto a : visited) {
		if (a == false)
			return false;
	}
	return true;
}

// This method will find the minimum spanning tree and calculate its weight which will be the
// actual output of program
void Graph::find_MST() {
	deque<int> vertex_q;
	int counter = 0;
	int v1 = 0, v2 = 0;
	// Let us start from 0 and insert it in Q
	vertex_q.push_back(0);
	visited.at(0) = true;

	while (!are_all_visited())
	{
		int min_weight = INT_MAX;
		int v1 = 0, v2 = 0;

		// for all vertices in queue
		for (auto int_v : vertex_q)
		{
			// for each vertex extracted find the min. weight. a will be pair with ->first as node number and ->second as weight
			for (auto a : graph.at(int_v)) {
				// Only check if node is not visited, then only check its weight...
				if (!visited.at(a.first) && a.second < min_weight) {
					v1 = int_v;
					v2 = a.first;
					min_weight = a.second;
				}
			}
		}

		// Now we have found the edge, mark v2 as visited and put v2 in Q and remove those edges
		// for all its adjacent vertices
		visited.at(v2) = true;
		vertex_q.push_back(v2);
		remove_edge(v1, v2);
		// Update the total weight of graph
		total_weight += min_weight;
		cout << "Edge # " << counter << " (" << v1 << ", " << v2 << ")" << "   with weight: " << min_weight << endl;
		++counter;
	}
}

// A method that will return the weight of MST
int Graph::get_total_weight() {
	return total_weight;
}

// Main function
void prim_mst_main(){
	srand(time(NULL));
	Graph mst_graph;
	mst_graph.find_MST();
	cout << "Number of vertices in graph are: " << mst_graph.get_num_of_vertices() << endl;
	cout << "Number of edges in graph are: " << mst_graph.get_num_of_edges() << endl;
	cout << "Weight of Minimum spanning tree is: " << mst_graph.get_total_weight();
	int ch = std::cin.get();
}

