// Implementation of Kosaraju's algorithm to print all SCCs
#include <algorithm>
#include <iostream>
#include <list>
#include <stack>
#include <fstream>
#include <sstream>
using namespace std;

//no,of vertices 
int counter[875714] = {0};
int index = 1;


class Graph
{
    int V;    // No. of vertices
    list<int> *adj;    // An array of adjacency lists

    // Fills Stack with vertices (in increasing order of finishing times)
    // The top element of stack has the maximum finishing time
    void fillOrder(int v, bool visited[], stack<int> &Stack);

    // A recursive function to print DFS starting from v
    void DFSUtil(int v, bool visited[], int u);
public:
    Graph(int V);
    void addEdge(int v, int w);

    // The main function that finds and prints strongly connected components
    void printSCCs();

    // Function that returns reverse (or transpose) of this graph
    Graph getTranspose();
};

Graph::Graph(int V)
{
    this->V = V;
    adj = new list<int>[V];
}

// A recursive function to print DFS starting from v
void Graph::DFSUtil(int v, bool visited[], int x)
{
    // Mark the current node as visited and print it
    visited[v] = true;
    //cout << v << " ";
    counter[x]++;

    // Recur for all the vertices adjacent to this vertex
    list<int>::iterator i;
    for (i = adj[v].begin(); i != adj[v].end(); ++i)
        if (!visited[*i])
            DFSUtil(*i, visited, x);
}

Graph Graph::getTranspose()
{
    Graph g(V);
    for (int v = 0; v < V; v++)
    {
        // Recur for all the vertices adjacent to this vertex
        list<int>::iterator i;
        for(i = adj[v].begin(); i != adj[v].end(); ++i)
        {
            g.adj[*i].push_back(v);
        }
    }
    return g;
}

void Graph::addEdge(int v, int w)
{
    adj[v].push_back(w); // Add w to list.
}

void Graph::fillOrder(int v, bool visited[], stack<int> &Stack)
{
    // Mark the current node as visited and print it
    visited[v] = true;

    // Recur for all the vertices adjacent to this vertex
    list<int>::iterator i;
    for(i = adj[v].begin(); i != adj[v].end(); ++i)
        if(!visited[*i])
            fillOrder(*i, visited, Stack);

    // All vertices reachable from v are processed by now, push v to Stack
    Stack.push(v);
}

// The main function that finds and prints all strongly connected components
void Graph::printSCCs()
{
    stack<int> Stack;

    // Mark all the vertices as not visited (For first DFS)
    bool *visited = new bool[V];
    for(int i = 0; i < V; i++)
        visited[i] = false;

    // Fill vertices in stack according to their finishing times
    for(int i = 0; i < V; i++)
        if(visited[i] == false)
            fillOrder(i, visited, Stack);

    // Create a reversed graph
    Graph gr = getTranspose();

    // Mark all the vertices as not visited (For second DFS)
    for(int i = 0; i < V; i++)
        visited[i] = false;

    // Now process all vertices in order defined by Stack
    while (Stack.empty() == false)
    {
        // Pop a vertex from stack
        int v = Stack.top();
        Stack.pop();

        // Print Strongly connected component of the popped vertex
        if (visited[v] == false)
        {
            gr.DFSUtil(v, visited, index);
	    ++index;
            //cout << endl;
        }
    }
}

// Driver program to test above functions
int main()
{	
	int i = 0, j= 0;

	string line = "";
	//1+number of vertices
	Graph g(875715);
	ifstream ifs;
	ifs.open("/home/pruthi/Desktop/SCC.txt");	
      // Create a graph given in the above diagram
	while (std::getline(ifs, line)) //Read a line 
	{
		std::stringstream ss(line);
		ss>>i>>j;
		g.addEdge(i, j);
		

	}
	

    cout << "Following are strongly connected components in given graph \n";
    g.printSCCs();
    std::sort(std::begin(counter), std::end(counter));
    cout<<"As per the sorting\n";
	// i = number of verrti - 1
    for(int i = 875713; i>=875680; --i){
		if(counter[i] == 0)
			continue;
		cout<<counter[i]<<", ";
	
    }

    return 0;
}
