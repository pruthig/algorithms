// Header file for Prim's MST algorithm, derived from Graph class

#ifndef _PRIM_H
#define _PRIM_H

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
#include "weighted_graph.h"

class Prim : public WeightedGraph {
	int total_weight;
public:
	Prim(char *file_name);
	void find_MST();
	int get_total_weight();
};

#endif