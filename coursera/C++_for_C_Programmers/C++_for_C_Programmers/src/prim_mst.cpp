
/*
* Prim's Minimum Spanning Tree algorithm.
*/

#include "prim_mst.h"

using namespace std;



// Constructor for the MST algorithm. It will call the Graph's constructor to fill the graph nodes
Prim::Prim(char *file_name) : WeightedGraph(file_name){
}

// This method will find the minimum spanning tree and calculate its weight which will be the
// actual output of program
void Prim::find_MST() {
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
int Prim::get_total_weight() {
	return total_weight;
}

// Main function
void prim_mst_main(){
	srand(time(NULL));
	Prim prim_mst("MST.txt");
	prim_mst.find_MST();
	cout << "Number of vertices in graph are: " << prim_mst.get_num_of_vertices() << endl;
	cout << "Number of edges in graph are: " << prim_mst.get_num_of_edges() << endl;
	cout << "Weight of Minimum spanning tree is: " << prim_mst.get_total_weight();
	int ch = std::cin.get();
}

