// Header file for hex class which will be derived from graph class

#ifndef _HEX_H
#define _HEX_H

#include<iostream>
#include<cstdlib>
#include<queue>

#include "computer.h"
#include "player.h"
#include "unweighted_graph.h"

enum class Color {
	BLUE,
	RED
};

class Hex : public UnweightedGraph {
	static const int DIM = 11;
	char graph_matrix[DIM][DIM];    // special data struct to display the board
	Player player;
	Computer computer;
public:
	Hex();
	void populate_graph(int dim);
	bool check_result(Color color);
	void display_board();
	// player refers either to 'H' (Human) or 'C' (Computer)
	bool make_move(int x, int y, char player);
	void update_matrix(int x, int y, char c);
	int get_board_dimension();
	Computer& computer_object();
	Player& player_object();
};

void hex_main();

#endif