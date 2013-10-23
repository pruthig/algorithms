//Creating base struct for graph  creation...

#include<iostream>

#define MAX_V 100
#define MAX_DEGREE 10

using namespace std;


struct graph;

void printGraph(struct graph *ptrGraph);
void insertEdges(struct graph *ptrGraph, int e1, int e2);


struct graph{
int edges[MAX_V+1][MAX_DEGREE];
int degree[MAX_V+1];
int n_v;
int n_e;
};


int main(){
struct graph g;

//Set degree to 0 initially for each vertex.. though we willn't use 0th index...
for(int i=0;i<MAX_V;i++)
	g.degree[i] = 0 ;

//End points of vertices and no. of vertices and edges respectively..
int end1 = 0, end2 = 0;
int n_v = 0, n_e = 0;

cout<<"Enter the number of vertices and edges in one line"<<endl;
cin>>g.n_v>>g.n_e;

cout<<"Now enter the edges in pairs "<<endl;

for(int i=0;i<g.n_e;i++){
	cin>>end1>>end2;
	insertEdges(&g, end1, end2);
}

printGraph(&g);

return 0;
}//End main


void insertEdges(graph *ptrGraph,int e1, int e2)
{
	ptrGraph->edges[e1][ptrGraph->degree[e1]] = e2;
	ptrGraph->edges[e2][ptrGraph->degree[e2]] = e1;

	ptrGraph->degree[e1]++;
	ptrGraph->degree[e2]++;
}

void printGraph(graph *ptrGraph)
{
	cout<<endl<<endl;
	for(int i=1;i<=ptrGraph->n_v;i++)
	{
		cout<<i;
		for(int j=0;j<ptrGraph->degree[i]; j++)
			cout<<" -> "<<ptrGraph->edges[i][j];
		cout<<endl;
	}//End for loop

}//End print function....





