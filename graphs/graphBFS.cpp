//Creating base struct for graph  creation...

#include<iostream>
#include<queue>

#define MAX_V 100
#define MAX_DEGREE 10

using namespace std;


struct graph;

void printGraph(struct graph *ptrGraph);
void insertEdges(struct graph *ptrGraph, int e1, int e2);
void bfsTraversal(struct graph *ptrGraph);
void process(int v);


struct graph{
int edges[MAX_V+1][MAX_DEGREE];
int degree[MAX_V+1];
bool discovered[MAX_V+1];
bool processed[MAX_V+1];
int n_v;
int n_e;
};

queue<int> q_graph;

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

//printGraph(&g);
bfsTraversal(&g);

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


void bfsTraversal(graph *ptrGraph)
{
	int vertex;                                     //vertex to be popped
	
	//mark all nodes not visited and not processe
	for(int i=1;i<=MAX_V;i++)
	{
		ptrGraph->discovered[i] = false;
		ptrGraph->processed[i] = false;
	}

	//find the starting vertex first..
	//Vertex index starts from 1 so i=1, don't access ptrGraph->degree[0]
	int start = 0;
	for(int i=1; i<MAX_V+1; i++){
		if(ptrGraph->degree[i] != 0) 
		{
			start = i;
			break;
		}
	}
	cout<<"Starting vertex :"<<start<<endl;

	q_graph.push(start);
	ptrGraph->discovered[start] = true;
	ptrGraph->processed[start] = true;
	process(start);
	
	while(!q_graph.empty())
	{
		vertex = q_graph.front();
		q_graph.pop();
		for(int i=0;i<ptrGraph->degree[vertex]; i++)                                              // 1->2 5
		{
			if(ptrGraph->discovered[ptrGraph->edges[vertex][i]] == false)                     // 2->1 3 
			{                                                                       // 3->2 4 5 6 
				q_graph.push(ptrGraph->edges[vertex][i]);                   // 4-> 3
				ptrGraph->discovered[ptrGraph->edges[vertex][i]] = true ;                        // 5->1 3
			}

		        if(ptrGraph->processed[ptrGraph->edges[vertex][i]] == false)                            // 6-> 3
			{
		 		process(ptrGraph->edges[vertex][i]);
                 		ptrGraph->processed[ptrGraph->edges[vertex][i]]  = true;
			}
		}//End for loop
	}
}//End bfs traversal

void process(int v){
 cout<<"Processed value is :"<<v<<"\n";
}

		
		
	


