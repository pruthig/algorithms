/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/
#include<iostream>
#include <list>
#include<vector>
#include<iterator>
#include<fstream>
#include<sstream>
#include <ctime>

using namespace std;

const int size = 200;
static std::vector< std::list<int>* > graph(size + 1);  // the 0th index won't be used.



static void construct_graph() {
	int num;
	int index = 0, counter = 0;
	std::ifstream infile("resources/MinCut.txt");

	std::string line;
	//std::vector< std::list<int> > vertex_list;
    // push NULL as first element so that we can access list from '1'
	graph.push_back(nullptr);
	while (getline(infile, line))
	{
		counter = 0;
		std::istringstream lineStream(line);
		while (lineStream >> num)
		{
			if (counter == 0)
			{
				index = num;
				counter = 1;
				continue;
			}
			if (graph.at(index) == NULL)
			{
				graph.at(index) = new std::list<int>();
			}
			graph.at(index)->push_back(num);
		}
	}


}


static int num_of_edges() {
	// graph is the vector of lists 
	int num = 0;
	for (auto list : graph) {
		if (list)
			num += list->size();
	}
	return num;
}

static int number_of_vertices(){
	int size = 0;
	for (auto ptr : graph)
	if (ptr) size++;

	return size;
}

void find_min_cut(){
	int num_edges = 0;
	int counter = 0;
	int v1 = 0, v2 = 0;
	while (number_of_vertices() >= 3) 
	{
		// pick and edge at random
		counter = 0;
		num_edges = num_of_edges();
		int random_number = (rand() % (num_edges)) + 1;
		for (std::vector< std::list<int>* >::iterator vecItr = graph.begin(); vecItr != graph.end(); ++vecItr) {
			if ((*vecItr) == NULL) continue;
			// a now is a std::list
			for (std::list<int>::iterator listItr = (*vecItr)->begin(); listItr != (*vecItr)->end(); ++listItr) {
				counter++;
				if (counter == random_number){
					v1 = vecItr - graph.begin();
					v2 = *listItr;
					goto DONE;
				}
			}
		}
		DONE:
		// Now, v1 and v2 are the vertices of edge to be merged
		// Conjoin, keeping the 1st one, by appending the elements of v2 to v1
		for (std::list<int>::iterator listItr = graph.at(v2)->begin(); listItr != graph.at(v2)->end(); ++listItr) {
			if (*listItr != v1) {
				graph.at(v1)->push_back(*listItr);
			}
		}
		// Remove v2 from the graph
		// Memory leak prevention code
		graph.at(v2) = NULL;
		// erase v2 entry from v1 list
		graph.at(v1)->remove(v2);
		// Replace all entries of v2 with v1
		for (std::vector< std::list<int>* >::iterator vecItr = graph.begin(); vecItr != graph.end(); ++vecItr) {
			if ((*vecItr) == NULL) continue;
			// a now is a std::list
			for (std::list<int>::iterator listItr = (*vecItr)->begin(); listItr != (*vecItr)->end(); ++listItr) {
				if (*listItr == v2)
					*listItr = v1;
			}
		}
	}

	// Calculate min cuts
	for (auto list : graph)
	if (list) {
		cout << "Min cut is: " << list->size() << endl;
		break;
	}

}

int kmc_main(){
	int count = 30;
	while (count--) 
	{
		// Feed the seed
		srand(time(NULL));
		construct_graph();
		find_min_cut();
	}
	return 0;



}

