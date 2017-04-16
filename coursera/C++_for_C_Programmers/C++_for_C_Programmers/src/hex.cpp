#include "hex.h"

using namespace std;


Hex::Hex() {
	// 2_D mx of chars - 'H' for human, 'C' for Computer, '0' means blank
	memset(graph_matrix, 'O', sizeof(graph_matrix[0][0])*DIM*DIM);
}

// Function that will update the board
// Return: true, if it was valid move, else false
bool Hex::make_move(int x, int y, char player) {
	if (x >= DIM || x < 0 || y >= DIM || y < 0) {
		cout << "Invalid move.";
		return false;
	}
	else {
		graph_matrix[x][y] = player;
		return true;
	}
}

void Hex::display_board() {
	cout << "                            Human" << endl << endl;
	for (int i = 0; i < DIM; ++i) {

		// Add spaces for character row...
		for (int m = 0; m <= i + 10; ++m) {
			if (i == DIM / 2 && m == i / 2 + 5) {
				cout << "Computer";
				m += 3;
			}
			cout << " ";
		}

		// Emit the character row...
		for (int j = 0; j < DIM; ++j) {
			cout << graph_matrix[i][j];
			if (j != DIM - 1)
				cout << " - ";
		}

		if (i == DIM / 2)
			cout << "    Computer";

		cout << endl;

		//Add spaces for slash row...
		for (int m = 0; m <= i + 11; ++m) {
			cout << " ";

		}

		// Enter the slash row...
		if (i < DIM - 1) {
			for (int k = 0; k < DIM; ++k) {
				cout << "\\ ";
				if (k != DIM - 1)
					cout << "/ ";
			}
		}

		cout << endl;
	}

	cout << endl << endl << "                                 Human";
}

void hex_main(){
	Hex hex_game;
	hex_game.display_board();
}
