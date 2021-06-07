#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <set>
#include <map>

#include "disjoint_set.h"

using namespace std; 

void countPathsUtil(int i, vector<int> adj[], int dst, int& count) {
    if(i == dst)
        ++count;
    for (int j : adj[i])
        countPathsUtil (j, adj, dst, count);
}

int countPaths(int V, vector<int> adj[], int source, int destination) {
    int path_count = 0;
    countPathsUtil(source, adj, destination, path_count);
    return path_count;
}

void numIslandsUtil(vector<vector<char>>& grid, bool *marked, int i, int j, int row, int col) {
    if(i<0 || j<0 || i>=row || j>=col || grid[i][j] == '0' || marked[i*col+j] == true)
        return;
    marked[i*col+j] = true;
    for(int m=-1;m<=1;++m) {
        for(int n=-1;n<=1;++n)
            numIslandsUtil(grid, marked, i+m, j+n, row, col);
    }
}

int numIslands(vector<vector<char>>& grid) {
    int row = grid.size();
    int col = grid[0].size();
    if(!row || !col)
        return 0;
    int count = 0;
    bool *marked = new bool[row*col];
    memset(marked, false, row*col);
    for(int i=0;i<grid.size();++i) {
        for(int j=0; j<grid[0].size(); ++j) {
            if(marked[i*col+j] == true || grid[i][j] == '0')
                continue;
            else {
                numIslandsUtil(grid, marked, i, j, row, col);
            }
            ++count;
        }
    }
    return count;
}

void DFSUtil (int i, vector <int> adj[], int V, bool vis[], vector <int> &res)
{
    if (vis[i]) return;
    vis[i] = true;
    res.push_back (i);
    
    for (int j : adj[i])
    {
        if (!vis[j]) DFSUtil (j, adj, V, vis, res);
    }
}

vector <int> dfsOfGraph(int V, vector<int> adj[])
{
    bool vis[V];
    memset (vis, false, sizeof (vis));

    vector <int> res;
    for (int i = 0;i < V; i++)
    {
        if (!vis[i])
        {
            DFSUtil (i, adj, V, vis, res);
        }
    }
    
    return res;
}

vector<int> bfsOfGraph(int V, vector<int> adj[]){
    vector<int> vec{};
    bool *marked = new bool[V];
    for(int i=0; i<V; ++i) {
        marked[i] = false;
    }
    
    queue<int> q{};
    q.push(0);
    marked[0] = true;
    while(!q.empty()) {
        int elem = q.front();
        vec.push_back(elem);
        q.pop();
       
        for(auto a: adj[elem]) {
            
            if(!marked[a]) {
                q.push(a);
                marked[a] = true;
            }
        }
    } 
    return vec;
}

// Cycle in directed graph
bool isCyclicUtil(int u, vector<int>adj[], vector<bool>& visited){
    for(auto v: adj[u]){
        if(visited[v])
            return true;
        else if(u == v)
            return false;
        if(isCyclicUtil(v, adj, visited))
            return true;
    }
    return false;
}

bool isCyclic(int V, vector<int>adj[]){
    vector<bool> visited(V, false);
    for(int i = 0; i < V; i++){
        if(!visited[i] && adj[i].size()>0){
            visited[i] = true;
            bool f = isCyclicUtil(i, adj, visited);
            if(f)return true;
        }
    }
    return false;
}

bool IsPathPossibleUtil(vector<vector<int>>& grid, bool *marked, int i, int j, int row, int col) {
    if(grid[i][j] == 1 && marked[i*col+j] == false) {
        return false;
    }
     
    if(grid[i][j] == 1 && marked[i*col+j] == true) {
        marked[i*col+j] = false;
    }
    
    if(i<0 || j<0 || i>=row || j>=col || grid[i][j] == 0 )// || marked[i*col+j] == true)
        return false;
    //marked[i*col+j] = true;
    if(grid[i][j] == 2)
        return true;
    else
        return (IsPathPossibleUtil(grid, marked, i, j-1, row, col) ||
            IsPathPossibleUtil(grid, marked, i, j+1, row, col) ||
            IsPathPossibleUtil(grid, marked, i-1, j, row, col) ||
            IsPathPossibleUtil(grid, marked, i+1, j, row, col));
}

bool IsPathPossible(vector<vector<int>>& grid) {
    int row = grid.size();
    int col = grid[0].size();
    if(!row || !col)
        return 0;

    bool *marked = new bool[row*col];
    memset(marked, false, row*col);
    for(int i=0;i<grid.size();++i) {
        for(int j=0; j<grid[0].size(); ++j) {
            if(grid[i][j] != 1)
                continue;
            else {
                marked[i*col+j] = true;
                return IsPathPossibleUtil(grid, marked, i, j, row, col);
            }
        }
    }
    return false;
}


void nearestUtil(vector<vector<int>>& grid, int i, int j, int row, int col, int temp, int& count, bool visit[]) {
        
    if(i<0 || j<0 || i>=row || j>=col)
        return;
    if(visit[i*col+j])
        return;;
        
    visit[i*col+j] = true;
    if(grid[i][j] == 1) {
        if(count > temp)
            count = temp;
        //visit[i*col+j] = false;
        return;
    }
    //if( 
    nearestUtil(grid, i, j-1, row, col, temp+1, count, visit);// ||
    visit[i*col+(j-1)] = false;
    nearestUtil(grid, i-1, j, row, col, temp+1, count, visit);
    visit[(i-1)*col+j] = false;
    nearestUtil(grid, i, j+1, row, col, temp+1, count, visit);// ||
    visit[i*col+(j+1)] = false;
    nearestUtil(grid, i+1, j, row, col, temp+1, count, visit);// )
    visit[(i+1)*col+j] = false;
    //return true;
}

vector<vector<int>> nearest(vector<vector<int>>grid){
    int row = grid.size();
    int col = grid[0].size();
    vector<vector<int>> res(row);
    bool *visit = new bool[row*col];
    memset(visit, false, row*col);
    
    if(!row || !col)
        return res;
    int count = 0;
    for(int i=0;i<row;++i) {
        vector<int> vec{};
        for(int j=0; j<col; ++j) {
            if(grid[i][j] == 1)
                vec.push_back(0);
            else {
                int count = INT_MAX;
                memset(visit, false, row*col);
                nearestUtil(grid, i, j, row, col, 0, count, visit);
                vec.push_back(count);
            }
        }
        res.push_back(vec);
    }
    return res;
}

struct comparator {
    bool operator()(pair< pair<int, int>, int>& p1, pair< pair<int, int>, int>& p2)
    {
        p1.second < p2.second;
    }
};

// Prim's algorithm
/* Optimal Approach: Since emphasis is on vertex, we create a set of vertices to check if a given vertex is included or not.
   For this, we create a set<int> and 2 more arrays, weight[] for storing the weight of each vertex (initial weight of each vertex
   is infinite) and another array parent[] to store parent. For the first vertex, we set its weight = 0 and push it to the priority queue.
   
   while !pq.empty:
        pick the vertex from pq.
        for each vertex 'v' adjacent to this vertex:
            If 'v' is not in MST and weight of (u,v) is smaller than current key of v, 
                update the value of weight[v]
                update the parent array as parent[v] = u
                add it to priority queue as <v, weight[v]>
*/            
   
int spanningTree(int V, vector<vector<int>> adj[]) {
    set<int> st{};
    vector<pair< pair<int, int>, int>> temp_vec{};
    priority_queue<pair< pair<int, int>, int>, vector<pair< pair<int, int>, int>>, comparator> pq{};

    int sum = 0;
    
    int core_vertex = 0, edge_vertex = 0;
    int weight_edge = INT_MAX;
    
    for(int i=0;i<V;++i) {
        for(int j=0; j<adj[i].size(); ++j) {
            vector<int> v = adj[i][j];
            //cout<<"i and j are: "<<i<<" "<<j<<endl;
            //cout<<"Pushed edge and weight:  ("<<i<<", "<<v[0]<<")  "<<v[1]<<endl;
            pq.push(make_pair(make_pair(i, v[0]), v[1]));
        }
    }

    pair< pair<int, int>, int > pr = pq.top();
    pq.pop();
    cout<<"Popped weight is: "<<pr.second<<endl;

    pair<int, int> vertices = pr.first;
    int weight = pr.second;
    sum += weight;
    st.insert(vertices.first);
    st.insert(vertices.second);
    
    //cout<<"Vertices are: "<<vertices.first<<" "<<vertices.second<<endl;
    //cout<<"Popped weight is: "<<pr.second<<endl;
    
    while(st.size() != V) {
        pair< pair<int, int>, int > pr = pq.top();
        pq.pop();
        pair<int, int> vertices = pr.first; 
        if((st.find(vertices.first) != st.end() && st.find(vertices.second) == st.end()) ||
           (st.find(vertices.second) != st.end() && st.find(vertices.first) == st.end()))
        {
            //cout<<"Vertices are: "<<vertices.first<<" "<<vertices.second<<endl;
            //cout<<"Popped weight is: "<<pr.second<<endl;
            sum += pr.second;
            if(st.find(vertices.second) == st.end())
                st.insert(vertices.second);
            else
                st.insert(vertices.first);
            
            // Push all vertices of vector
            while(!temp_vec.empty()) {
                pair< pair<int, int>, int > pt = temp_vec.back();
                temp_vec.pop_back();
                pq.push(pt);
            }
            continue;
        }
        else 
        {
            temp_vec.push_back(pr);
        }
                    
    }
    return sum;
}

bool comparator_kruskal(pair< pair<int, int>, int>& p1, pair< pair<int, int>, int>& p2) {
    p1.second < p2.second;
}

int Kruskal(int V, vector<vector<int>> adj[]) {
    
    int sum = 0;
    vector<pair< pair<int, int>, int>> edges{};
    
    for(int i=0;i<V;++i) {
        for(int j=0; j<adj[i].size(); ++j) {
            vector<int> v = adj[i][j];
            edges.push_back(make_pair(make_pair(i, v[0]), v[1]));
        }
    }
    // Kruskal is all about sorting and edges and picking an edge wherein
    // its ends are not a part of same set.
    // vector of edges -> sort them -> traverse and check for presence in set.
    sort(edges.begin(), edges.end(), comparator_kruskal);
    
    // Create disjoint set
    DisjointSet disjointSet;
    disjointSet.make_set(V);
    
    for(auto a : edges) {
        int v1 = a.first.first;
        int v2 = a.first.second;
        if(disjointSet.find_parent(v1) == disjointSet.find_parent(v2))
            continue;
        else {
            sum += a.second;
            disjointSet.union_set(v1, v2);
        }
    }
    return sum;
}
    
// Now edges are sorted. Pick one edge and 

// Dijkstra algorithm is similar to prim's algorithm in way that we use heap to find the weight of target vertex.
// We will have a bool array - visited[] , priority_queue<pair<node, weight>> pq{}. We will maintain verties weight 
// in the form of INFINITY
// Mark first as source vertex.
// for all vertices adjacent to it && not marked as visited:
//      make a check whether weight[dst] > weight[edge.src] + edge_weight 
//      update the weight in case above condition is true
// Mark the source vertex as true
// Run extract_min on heap and make it as source vertex.
// Use visited[] array to check marking of node in O(1) time and
// priority_queue for updating the weight value.. for this we need to create custom heap
// to reduce the weight of the vertex after which we'll call heapify method.
// We can create another map path_map<node, parent> to find the path

/* ----------------------------------------------------------------------------------------------- */

// Bellman - ford algorithm 
// Store all the edges
// for i=0 to V-1 (need to repeat V-1 times)
//   for each edge:
//      if weight[edge.src] + edge_weight < weight[edge.dst]
//         weight[edge.dst] = weight[edge.src] + edge_weight
// End
// Bellman ford algorithm won't work in case we have negative weight cycles.
// To find minimum number of jumps required to make source word equal to target word.
// Use BFS using queue, put source word in queue and,now, pop the word from queue, generate
// all its permutations which differ only by 1 character and push in queue. Before pushing make sure that the interim element
// exists in dictionary and delete from dictionary just before pushing to avoid cycles.

/* Shortest path to reach one prime to other by changing single digit at a time
// Generate all primes using sieve of Eratosthenes... O(n*log(log(n))) 
// create a graph using those numbers... and using BFS, find the steps need to be jumped from 1 no. to another
// 

int main() {
    //vector<vector<char>> vecvec { {'0','1','0','0','1','1','0','0'}, {'1','0','1','1','0','0','1','1'}, {'0','1','1','1','0','0','0','1'} };
    //vector<vector<int>> checkPathVec { { 3,0,3,0,0 }, { 3,0,0,0,3 }, { 3,3,3,3,3 }, { 0,2,3,0,0 }, { 3,0,0,1,3 } };
    /* vector<vector<int>> checkPathVec {{ 3,3,3,3,0,0,3,0 },
                                      { 1,3,3,3,3,3,3,2 },
                                      { 3,3,0,3,0,3,3,3 },
                                      { 3,3,3,0,0,3,3,0 },
                                      { 0,3,3,3,3,3,3,3 },
                                      { 0,0,0,3,3,0,3,3 },
                                      { 0,3,0,3,3,3,3,0 },
                                      { 3,3,3,0,3,3,3,3 }};
    vector<int> adj[5];
    adj[0].push_back(1);
    adj[1].push_back(2);
    adj[2].push_back(3);
    adj[3].push_back(4);
    */
    //vector<vector<int>>  vecvec { {0,1,0,0,1,1,1,1}, {0,0,1,0,1,1,0,1}, {1,0,0,0,1,0,0,0} };
    //vector<vector<int>> res{};
    //cout<<"Number of islands are: "<<numIslands(vecvec)<<endl;
    //cout<<"Is graph cyclic: "<<isCyclic(4, adj)<<endl;
    //cout<<IsPathPossible(checkPathVec)<<endl;
    //res = nearest(vecvec);
    
    return 0;
}