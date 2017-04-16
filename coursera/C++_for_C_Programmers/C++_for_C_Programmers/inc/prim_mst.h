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
#include "graph.h"

class Prim : public Graph {
public:
	Prim(char *file_name);
	void find_MST();
};

#endif