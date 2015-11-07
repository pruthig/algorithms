#include "graphAdjList.h"

void Graph::init(int a){
    vertexCount = a;
    marked = new bool[a];
    edgeTo = new int[a];
    id = new int[a];
    counter = 0;
    for(int i = 0 ; i < a; ++i)
        marked[i] = false;
    addVertex(a);
}

int Graph::degree(int i) {
  return graphVector[i].size();
}

void Graph::addEdge(int v, int w) {
  graphVector[v].push_back(w);
}

bool Graph::deleteEdge(int v, int w) {
  vector<int>::iterator itr;
  if(searchEdge(v, w) != graphVector[v].end()) {
    graphVector[v].erase(itr);
    return true;
  }
  return false;
}

vector<int>::iterator Graph::searchEdge(int v, int w) {
  vector<int>::iterator itr;
  for(itr = graphVector[v].begin(); itr != graphVector[v].end(); ++itr) {
      if(*itr == w) {
        return itr;
    }   
    return graphVector[v].end();
}
}

void Graph::addVertex(int v) {
    for(int i = 0; i < v; ++i) {
        vector<int> a;
        a.clear();
    	graphVector.push_back(a);
    }
}

void Graph::dfs(int vertex) {

    if(!marked[vertex]) {
        marked[vertex] = true;
        std::cout<<"\nw = "<<vertex<<" marked true"<<endl;
    }
    id[vertex] = counter;
    vector<int> vect = graphVector[vertex];
    for(int w : vect) {
        if(!marked[w]) {
            marked[w] = true;
            std::cout<<"\nw = "<<w<<" marked true"<<endl;
            dfs(w);
        }
    }
}

void Graph::dfsWithStack(int vertex) {
    stack<int> stck;
    
    stck.push(vertex);
    while(!stck.empty()) {
        int m = stck.top();
        if(marked[m] == true) {
            stck.pop();
            continue;
        }

        marked[m] = true;
        std::cout<<"\nm = "<<m<<" marked true"<<endl;
        stck.pop();
        vector<int> vect = graphVector[m];

        for(int w : vect) {
            
            if(!marked[w]) {
                stck.push(w);
            }
        }

    }
}

//Do the bfs of graph With Q
void Graph::bfsWithQueue(int vertex) {
    queue<int> q;
    
    q.push(vertex);
    marked[vertex] = true;
    while(!q.empty()) {
        int m = q.front();
        std::cout<<"\nm = "<<m<<" marked true"<<endl;
        q.pop();
        vector<int> vect = graphVector[m];

        for(int w : vect) {
            
            if(!marked[w]) {
                marked[w] = true;
                edgeTo[w] = m;
                q.push(w);
            }
        }

    }
}


void Graph::cc() {
    for(int  i = 0; i < vertexCount; ++i) {
        if(!marked[i]) {
            dfs(i);
            ++counter;
        }
    }
}


//To cover DFS we need to have double entries for edges: x,y and y,x
int main(){
	Graph gObject;
    gObject.init(10);

	gObject.addEdge(0, 5);
	gObject.addEdge(0, 6);

	gObject.addEdge(1, 0);

	gObject.addEdge(2, 3);
	gObject.addEdge(2, 1);

	gObject.addEdge(3, 4);

	gObject.addEdge(4, 9);

	gObject.addEdge(5, 6);

	gObject.addEdge(6, 1);
	gObject.addEdge(6, 7);
	gObject.addEdge(6, 2);

	gObject.addEdge(7, 2);


	gObject.addEdge(8, 3);
	gObject.addEdge(8, 9);
	gObject.addEdge(8, 2);
	gObject.addEdge(8, 7);

	gObject.addEdge(9, 3);

    gObject.bfsWithQueue(0);
	return 0;
}
