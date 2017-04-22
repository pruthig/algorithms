#include "hex.h"

using namespace std;

// Default constructor to initialize graph
Hex::Hex():UnweightedGraph(DIM*DIM)
{
	// 2_D mx of chars - 'B' for Blue, 'R' for Red, '0' means blank
	// Here we are blanking out the matrix
	memset(graph_matrix, 'O', sizeof(graph_matrix[0][0])*DIM*DIM);
	populate_graph(DIM);
}

int Hex::get_board_dimension() {
	return DIM;
}

// This method will update the adjacency list representation of graph with nodes
// accompanied with their neighbours
void Hex::populate_graph(int dim) {

	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			// top left - 2 connections
			if (i == 0 && j == 0) {
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);
				graph[DIM * i + j].push_back(DIM * i + j + 1);
			}
			// bottom right - 2 connections
			else if (i == DIM - 1 && j == DIM - 1){
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * i + j - 1);
			}
			// right top..3 connections
			else if (i == 0 && j == dim - 1) {
				graph[DIM * i + j].push_back(DIM * i + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);
			}
			// left bottom...3 connections
			else if (i == dim - 1 && j == 0) {
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j + 1);
				graph[DIM * i + j].push_back(DIM * i + j + 1);
			}
			// left edge..4 connections
			else if (j == 0) {
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j + 1);
				graph[DIM * i + j].push_back(DIM * i + j + 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);

			}
			// top edge .. 4 connections
			else if (i == 0) {
				graph[DIM * i + j].push_back(DIM * i + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);
				graph[DIM * i + j].push_back(DIM * i + j + 1);

			}
			// right edge .. 4 conns
			else if (j == DIM - 1) {
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * i + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);

			}
			// bottom edge..4 connections
			else if (i == DIM - 1) {
				graph[DIM * i + j].push_back(DIM * i + j - 1);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j + 1);
				graph[DIM * i + j].push_back(DIM * i + j + 1);
			}
			// 4 corners and 4 edges are covered..rest all have all connected
			else {
				graph[DIM * i + j].push_back(DIM * i + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j - 1);
				graph[DIM * i + j].push_back(DIM * (i + 1) + j);
				graph[DIM * i + j].push_back(DIM * i + j + 1);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j);
				graph[DIM * i + j].push_back(DIM * (i - 1) + j + 1);
			}


		}
	}
}

// Check i
bool Hex::check_result(Color color) {
	// Queue to traverse the path completed by the Computer
	deque<int> comp_path;
	// Vector to keep track of visited nodes
	vector<bool> visited(DIM*DIM, false);

	//char to match for player
	char matching_char;

	// set the matching char
	if (color == Color::BLUE)
		matching_char = 'B';
	else
		matching_char = 'R';

	// tmp element to traverse
	int tmp_int = 0;
	int x = 0, y = 0;
	for (int i = 0; i < DIM; ++i) {
		if ((color == Color::BLUE && graph_matrix[0][i] == 'B') || (color == Color::RED && graph_matrix[0][i] == 'R')) {
			comp_path.push_back(DIM * i);
		}
	}
	// search for path by doing BFS
	while (!comp_path.empty()) {
		tmp_int = comp_path.front();
		// Check for the end point of path
		if ((color == Color::BLUE && tmp_int%DIM == DIM - 1) || (color == Color::RED && (tmp_int / DIM == DIM - 1)))
			return true;
		comp_path.pop_front();
		visited.at(tmp_int) = true;
		for (auto a : graph.at(tmp_int)) {
			//dig x and y coordinates
			x = a / DIM;
			y = a % DIM;
			// push the node not visited yet
			if (graph_matrix[x][y] == matching_char && visited.at(a) == false)	
				comp_path.push_back(a);
		}
	}
	return false;
}

void Hex::update_matrix(int x, int y, char c) {
	graph_matrix[x][y] = c;
}



void Hex::display_board() {
	std::cout << "                            Human" << endl << endl;
	for (int i = 0; i < DIM; ++i) {

		// Add spaces for character row...
		for (int m = 0; m <= i + 10; ++m) {
			if (i == DIM / 2 && m == i / 2 + 5) {
				std::cout << "Computer";
				m += 8;
			}
			std::cout << " ";
		}

		// Emit the character row...
		for (int j = 0; j < DIM; ++j) {
			std::cout << graph_matrix[i][j];
			if (j != DIM - 1)
				std::cout << " - ";
		}

		if (i == DIM / 2)
			std::cout << "    Computer";

		std::cout << endl;

		//Add spaces for slash row...
		for (int m = 0; m <= i + DIM; ++m) {
			std::cout << " ";

		}

		// Enter the slash row...
		if (i < DIM - 1) {
			for (int k = 0; k < DIM; ++k) {
				std::cout << "\\ ";
				if (k != DIM - 1)
					std::cout << "/ ";
			}
		}

		std::cout << endl;
	}

	std::cout << endl << endl << "                                 Human";
}

//void Hex::check_who_won() {
//	if (check_result(this->computer_color)) {
//		std::cout << "Computer won"<<endl;
//		return;
//	}
//	if (check_result(Color::BLUE)) {
//		std::cout << "Human won"<<endl;
//		return;
//	}
//	std::cout << "Game not ended yet."<<endl;
//}

// Set color choice for computer
//void Hex::set_my_color(Color c) {
//	this->color_choice = c;
//}

// return the contained object
Computer& Hex::computer_object(){
	return this->computer;
}

Player& Hex::player_object(){
	return this->player;
}

int main(){
	int option;
	Hex hex_game;
	int choice = 0;
	bool check_comp_won = false;
	cout << "**************** Welcome to the game of Hex ***************" << endl;
	hex_game.display_board();
	while(1) {
		cout << "Choose your color:\n Enter 1 for RED and 2 for BLUE\n";
		cin >> choice;
		if (choice != 1 && choice != 2)
			cout << "Invalid choice, please try again";
		else
			break;
	}

	if (choice == 1) {
		hex_game.computer_object().set_color(Color::BLUE);
		hex_game.player_object().set_color(Color::RED);
		cout << "My color is BLUE and I will play the first turn" << endl;
		hex_game.computer_object().make_move(hex_game);
	}
	else {
		hex_game.computer_object().set_color(Color::RED);
		hex_game.player_object().set_color(Color::BLUE);
		cout << "Congrats you'll play first" << endl;
	}

	while (1) {
		std::cout << endl<<endl<<"Select the option " << endl;
		std::cout << "1. Make a move" << endl;
		std::cout << "2. Exit" << endl;
		cin.clear();
		cin >> option;
		switch (option) {
		case 1:
			hex_game.player_object().make_move(hex_game);
			break;
		case 2:
			exit(0);
			break;
		default:
			std::cout << "Try again." << endl;
			break;

		}
	}
	return 0;
}
