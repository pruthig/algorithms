#include "graphAdjList.h"

using std::vector;

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
	vector<int> a;
	graphVector.push_back(a);
}


int main(){
	Graph gObject;
	vector<int> v;
	gObject.addVertex(1);
	gObject.addVertex(2);

	gObject.addEdge(0, 2);
	gObject.addEdge(1, 2);
	return 0;
}
