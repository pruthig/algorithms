// Header file for hex class which will be derived from graph class

#ifndef _HEX_H
#define _HEX_H

#include<iostream>
#include<cstdlib>

#include "graph.h"

enum class GAME_STATUS {
	BLUE_WON,
	RED_WON,
	IN_PROGRESS
};
class Hex : public Graph {
	static const int DIM = 11;
	char graph_matrix[DIM][DIM];    // special data struct to display the board
public:
	Hex();
	void display_board();
	// player refers either to 'H' (Human) or 'C' (Computer)
	bool make_move(int x, int y, char player);
	GAME_STATUS find_game_status();
};

void hex_main();

#endif