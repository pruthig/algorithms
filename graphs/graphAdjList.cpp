#include "graphAdjList.h"

int Graph::degree(int i) {
  return graph[i].size();
}

void Graph::addEdge(int v, int w) {
  graph[v].push_back(w);
}

bool Graph::deleteEdge(int v, int w) {
  vector<int>::iterator itr;
  if(searchEdge(v, w) != graph[v].end()) {
    graph[v].erase(itr);
    return true;
  }
  return false;
}

vector<int>::iterator Graph::searchEdge(int v, int w) {
  vector<int>::iterator itr;
  for(itr = graph[v].begin(); itr != graph[v].end(); ++itr) {
      if(*itr == w) {
        return itr;
    }   
    return graph[v].end();
}
}


int main(){
	return 0;
}
