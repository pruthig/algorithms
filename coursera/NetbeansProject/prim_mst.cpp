
/*
* Prim's Minimum Spanning Tree algorithm.
*/

#include<iostream>
#include <list>
#include<vector>
#include<fstream>
#include<utility>
#include<sstream>
#include<queue>
#include <ctime>
#include <climits>
#include <cstdlib>

using namespace std;

// Main Graph class to implement the Prims' algorithm to find the minimum spanning tree and
// calculate its weight.
class Graph_MST{
private:
	vector< std::list< pair<int, int> >* > *graph;
	vector<bool> *visited;
	int total_weight;
public:
	Graph_MST();
	~Graph_MST();
	//void add_edge(int u, int v, int w);
	int num_of_edges();
	int num_of_vertices();
	int get_total_weight();

	bool are_all_visited();
	void find_MST();

};

// Constructor for the MST algorithm. It will read the file which contains the edges with weights.
Graph_MST::Graph_MST() {
	// read graph from a file
	int graph_size = 0;
	int counter = 0;
	std::ifstream infile("MST.txt");

	std::string line;
	int vertex1 = 0, vertex2 = 0, edge_weight = 0;
	while (getline(infile, line))
	{
		std::istringstream lineStream(line);
		// first line mentions the size of graph..read and continue.
		if (counter == 0) {
			counter = 1;
			lineStream >> graph_size;
			graph = new vector< std::list< pair<int, int> >* >(graph_size);
			visited = new vector<bool>(graph_size, false);
			continue;
		}
		//   
		lineStream >> vertex1 >> vertex2 >> edge_weight;
		if (graph->at(vertex1) == NULL)
		{
			graph->at(vertex1) = new std::list< pair<int, int> >();
		}
		graph->at(vertex1)->push_back(std::make_pair(vertex2, edge_weight));

	}
	// Counter to sum up the minimum weight of graph
	total_weight = 0;
}

// Destructor
Graph_MST::~Graph_MST() {
	delete visited;
	// delete all the adjacent edges
	for (auto a : *graph) {
		if (a)
			a->clear();
	}

    (*graph).clear();
	graph = nullptr;
}

// Returns the number of edges..
int Graph_MST::num_of_edges() {
	// graph is the vector of lists 
	int num = 0;
	for (auto list : *graph) {
		if (list)
			num += list->size();
	}
	return num;
}

// Returns the number of vertices
int Graph_MST::num_of_vertices(){
	int size = 0;
	for (auto ptr : *graph)
	if (ptr) size++;

	return size;
}

// Method to check if all vertices are visited.
bool Graph_MST::are_all_visited() {
	for (auto a : *visited) {
		if (a == false)
			return false;
	}
	return true;
}

// This method will find the minimum spanning tree and calculate its weight which will be the
// actual output of program
void Graph_MST::find_MST() {
	deque<int> vertex_q;
	int num_edges = 0;
	int counter = 0;
	int v1 = 0, v2 = 0;
	// Let us start from 0 and insert it in Q
	vertex_q.push_back(0);
	visited->at(0) = true;
	while (!are_all_visited())
	{
		int min_weight = INT_MAX;
		int v1 = 0, v2 = 0;

		// for all vertices in queue
		for (auto int_v : vertex_q)
		{
			// Extract the vertex whose adjacent vertices need to be searched...
			v1 = int_v;
			// for each vertex extracted find the min. weight. a will be pair with ->first as node number and ->second as weight
			for (auto a : *(graph->at(v1))) {
				// Only check if node is not visited, then only check its weight...
				if (!visited->at(a.first) && a.second < min_weight) {
					min_weight = a.second;
					v2 = a.first;
				}
			}
		}
		
		// Now we have found the edge, mark v2 as visited and put v2 in Q and remove those edges
		// for all its adjacent vertices
		visited->at(v2) = true;
		vertex_q.push_back(v2);
		graph->at(v1)->remove(std::make_pair(v2, min_weight));
		graph->at(v2)->remove(std::make_pair(v1, min_weight));
		// Update the total weight of graph
		total_weight += min_weight;
	}
}

// A method that will return the weight of MST
int Graph_MST::get_total_weight() {
	return total_weight;
}

// Main function
void prim_mst_main(){
	srand(time(NULL));
	Graph_MST mst_graph;
	mst_graph.find_MST();
	cout<<"Weight of Minimum spanning tree is: "<<mst_graph.get_total_weight();
}

