#include"dijkstra.h"

using namespace std;

void split(const string& str, const string& delim, vector<string>& parts) {
	size_t start, end = 0;
	while (end < str.size()) {
		start = end;
		while (start < str.size() && (delim.find(str[start]) != string::npos)) {
			start++;  // skip initial whitespace
		}
		end = start;
		while (end < str.size() && (delim.find(str[end]) == string::npos)) {
			end++; // skip to end of word
		}
		if (end - start != 0) {  // just ignore zero-length strings.
			parts.push_back(string(str, start, end - start));
		}
	}
}

void Dijkstra::construct_graph() {
	string str;
	int index = 0, counter = 0;
	std::ifstream infile("resources/dijkstraData.txt");

	std::string line;
	std::vector<string> temp_vec;
	// The size of vector is - requisite number of nodes + 1, so as to skip 0th index
	graph.resize(NUM_VERTICES+1);
	while (getline(infile, line))
	{
		counter = 0;
		std::istringstream lineStream(line);
		while (lineStream >> str)
		{
			if (counter == 0)
			{
				index = stoi(str);
				counter = 1;
				continue;
			}
			split(str, ",", temp_vec);
			graph.at(index).push_back(make_pair( stoi(temp_vec[0]), stoi(temp_vec[1]) ) );
			temp_vec.clear();
		}
	}
	// It will store the distance of node from start
	dist[0] = 0;
	for (int i = 1; i <= NUM_VERTICES; ++i) {
		dist[i] = INT_MAX;
	}
	// Update for 1st
	dist[1] = 0;
	// It will store previous node index
	std::memset(prev, 0, sizeof(prev));

} // Done constructing graph

void Dijkstra::calculate_shortest_path() {
	while (heap.size() > 0) {
		node top_element = heap.top();
		heap.pop();
		// loop over all edges of popped node.. 
		for (auto a : graph.at(top_element.index)) {
			//(u, v)--> (top_element.index, a.first)
			if (dist[top_element.index] + a.second < dist[a.first]) {
				dist[a.first] = dist[top_element.index] + a.second;
				prev[a.first] = top_element.index;
				node n = { a.first, dist[a.first] };
				heap.push(n);

			}
		}
	}
}
void Dijkstra::init_heap() {
	// Node is of type = { node_index, distance_from_start };
	// Add vertex 1 with distance 0 to heap
	node x = { 1, 0 };
	heap.push(x);
	for (int i = 2; i <= NUM_VERTICES; ++i) {
		node y = { i, INT_MAX };
		heap.push(y);
	}
}

void Dijkstra::print_node_distance() {
	for (int i = 1; i <= NUM_VERTICES; ++i) {
		cout << "Distance of vertex " << i << " is " << dist[i] << endl;
	}
	cout << endl;
}
// Main function to invoke Dijkstra algo.
void dijkstra_main() {
	Dijkstra d;
	d.construct_graph();
	d.init_heap();
	d.calculate_shortest_path();
	d.print_node_distance();
}
