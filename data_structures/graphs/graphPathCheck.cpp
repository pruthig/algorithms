// This graph checks if there exists path between 2 given vertices in a matrix, given that rest of cells depict either
// a wall or blank cell... Value of cell 1 means Source, 2 means Destination, 3 means blank cell and 0 means blank wall
// BFS will be used to achieve the motive...
#include<iostream>
#include<queue>
#define MAX_V 5

using namespace std;



namespace {
	int graph[MAX_V][MAX_V] = 		{
									   {3, 3, 0, 2, 0},
					                   {3, 3, 0, 0, 3},
					                   {3, 0, 3, 3, 3},
					                   {3, 0, 1, 0, 0},
					                   {0, 0, 3, 0, 3} 
					               };
					               
	// src [3, 2] dest [0, 3]
	bool visited[MAX_V][MAX_V] = { false, };
	struct node {
		int x, y; //coordinates
	};
	queue<node> q_n;
	
}

bool isValid(int i, int j) {
	if(i < 0 || j < 0 || i >= MAX_V ||  j >= MAX_V || graph[i][j] == 0)
		return false;
	return true;
}
	
void updateQWithNeighbors(int i, int j) {
	if(isValid(i, j) && visited[i][j] == false)	{
		struct node nd = { i, j };
		q_n.push(nd);
		visited[i][j] = true;
	}
}

void searchPathUtil(node& n) {
	if(!isValid(n.x, n.y)) {
		cout<<"\nInvalid start node\n";
		return;
	}

	visited[n.x][n.y] = true;
	
	q_n.push(n);
	
	while(!q_n.empty()) {
		node t = q_n.front();
		if(graph[t.x][t.y] == 2) {
			cout<<"Found a valid path\n";
			return;
		}	
		q_n.pop();
		// Add neighbors to queue...
		// top row
		updateQWithNeighbors(t.x-1, t.y-1);
		updateQWithNeighbors(t.x-1, t.y);
		updateQWithNeighbors(t.x-1, t.y+1);
		// middle row
		updateQWithNeighbors(t.x, t.y-1);
		updateQWithNeighbors(t.x, t.y+1);
		// bottom row
		updateQWithNeighbors(t.x+1, t.y-1);
		updateQWithNeighbors(t.x+1, t.y);
		updateQWithNeighbors(t.x+1, t.y+1);
	}
	
}


int main(){
	struct node start = { 3, 2 };
	searchPathUtil(start);
	return 0;
}

