#include "player.h"
#include "hex.h"

// Function that will update the board
// Return: true, if it was valid move, else false
void Player::make_move(Hex& hex_game) {
	int x = 0, y = 0;
	char player;
	int DIM = hex_game.get_board_dimension();
	std::cout << "Enter the following - x-coord y-coord player-in-char-form" << endl;
	cout << "Player: enter 'c' for computer and 'h' for human." << endl;
	cout << "Example: To place a piece from computer context at (0,0) input would be: 0 0 c " << endl;
	cin >> x >> y >> player;
	if (x >= DIM || x < 0 || y >= DIM || y < 0 || !isalpha(player) || (player != 'c' && player != 'h')) {
		std::cout << "Invalid move.";
	}
	else {
		hex_game.update_matrix(x, y, player);
	}
}