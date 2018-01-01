// This graph finds the number of islands in a matrix. An island is the maximum number of 1's 
// in a given area...
#include<iostream>

#define MAX_V 5


using namespace std;


namespace {
	int graph[MAX_V][MAX_V] = 		{
									   {1, 1, 0, 0, 0},
					                   {0, 1, 0, 0, 1},
					                   {1, 0, 0, 1, 1},
					                   {0, 0, 0, 0, 0},
					                   {1, 0, 1, 0, 1} 
					               };
	bool visited[MAX_V][MAX_V] = { false, };
}

void DFSUtil(int i, int j) {
	if(i < 0 || j < 0 || i >= MAX_V ||  j >= MAX_V || graph[i][j] == 0 || visited[i][j] == true)
		return;
	visited[i][j] = true;
	// call DFSUtil for all nearby vertices (8 in total)
	
	// top row
	DFSUtil(i-1, j-1);
	DFSUtil(i-1, j);
	DFSUtil(i-1, j+1);
	// middle row
	DFSUtil(i, j-1);
	DFSUtil(i, j+1);
	// bottom row
	DFSUtil(i+1, j-1);
	DFSUtil(i+1, j);
	DFSUtil(i+1, j+1);
	
}
int DFS() {
	int count_i = 0;
	for(int i = 0; i < MAX_V; ++i) {
		for(int j = 0; j < MAX_V; ++j) {
			if(graph[i][j] == 1 && !visited[i][j]) {
				DFSUtil(i, j);
				++count_i;
			}
		}
	}
	return count_i;
		
}

int main(){
	cout<<"Number of islands are: "<<DFS()<<endl;
	return 0;
}

