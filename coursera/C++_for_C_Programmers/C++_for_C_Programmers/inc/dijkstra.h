#ifndef _DIJKSTRA_H
#define _DIJKSTRA_H

#include "weighted_graph.h"
#include<queue>
#include<limits>
#include<functional>

// This class defines the methods to be used in dijkstra shortest path algorithm.
// It is derived from the WeightedGraph class
class Dijkstra : public WeightedGraph {
	static const int NUM_VERTICES = 200;
	struct node {
		int index;
		int weight;
	};
	// functor for heap
	class Compare
	{
	public:
		bool operator() (node n1, node n2) {
			return n1.weight > n2.weight;
		}
	};

	// priority queue of node of integers...
	std::priority_queue<node, vector<node>, Compare> heap;
	int dist[NUM_VERTICES + 1];
	int prev[NUM_VERTICES + 1];
public:

	void construct_graph();
	void calculate_shortest_path();
	void init();
	void init_heap();
	void print_node_distance();
};

#endif