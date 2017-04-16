#include<iostream>
#include<cstring>
#include <cstdlib>


using namespace std;


#define DIM 11

void displayGraph();

// Two data structures will be used
// A 2-D array for display and storing the values
char graph[DIM][DIM];
// and adjacency list representation of graph


void hex_main() {
	// 2_D mx of chars - 'H' for human, 'C' for Computer, '0' means blank
	memset(graph, 'O', sizeof(graph[0][0])*DIM*DIM);
	int x, y;
	char c;
	displayGraph();
	cout << endl << endl;
	while (1) {
		cout << "Enter the coordinates" << endl;
		cin >> x >> y >> c;
		if (x >= DIM || x < 0 || y >= DIM || y < 0) {
			cout << "Invalid input\n";
		}
		else {
			graph[x][y] = c;
		}
		displayGraph();
	}
}

void displayGraph() {
	cout << "                            BLUE" << endl << endl;
	for (int i = 0; i < DIM; ++i) {

		// Add spaces for character row...
		for (int m = 0; m <= i + 10; ++m) {
			if (i == DIM / 2 && m == i / 2 + 5) {
				cout << "RED";
				m += 3;
			}
			cout << " ";
		}

		// Emit the character row...
		for (int j = 0; j < DIM; ++j) {
			cout << graph[i][j];
			if (j != DIM - 1)
				cout << " - ";
		}

		if (i == DIM / 2)
			cout << "    RED";

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

	cout << endl << endl << "                                 BLUE";
}

