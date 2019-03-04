// Each added edge is directed
#include<iostream>
#include<memory>
#include<set>
#include<vector>
#include<stack>
#include<queue>

using namespace std;


class Graph
{
    vector<vector<int>> graph{};
    int num_of_vertices;
public:
    Graph(int n)
    {
        num_of_vertices = n;
        graph.reserve(n);
        for(int i = 0; i < n; ++i)
        {
            vector<int> vec{};
            graph.push_back(vec);
        }
    }
    void addEdge(int a, int b)
    {
        graph[a].push_back(b);
    }
    void BFS();
    void checkCycle(int vertex, bool* visited, set<int>& white,set<int>&  black, set<int>& gray);
    void detectCycleDirected();
};

void Graph::BFS()
{
    queue<int> q{};
    bool visited[num_of_vertices] = {false,};
    q.push(0);
    visited[0] = true;

    while(!q.empty())
    {
        int num = q.front();
        q.pop();

        visited[num] = true;
        cout<<"Visited: "<<num<<endl;

        for(auto& a : graph.at(num))
        {
            if(visited[a] == false)
                q.push(a);
        }
    }
}

void Graph::checkCycle(int vertex, bool* visited, set<int>& white,set<int>&  black, set<int>& gray)
{
    for(int i = 0; i < graph.at(vertex).size(); ++i)
    {
        int a = graph.at(vertex)[i];
        if(!visited[a])
        {
            white.erase(a);
            gray.insert(a);
            visited[a] = true;
        }
        else
        {
            if(gray.find(a) != gray.end())
            {
                cout<<"Cycle detected\n";
                return;
            }
        }
        checkCycle(a, visited, white, black, gray);
        gray.erase(a);
        black.insert(a);
    }
}

void Graph::detectCycleDirected()
{
    // create 3 sets - white(not yet encountered), black(visited), gray(currently under consideration)
    set<int> white{}, black{}, gray{};
    bool *visited = new bool[graph.size()];
    std::fill(visited, visited+graph.size(), false);

    for(auto& a : graph)
    {
        static int count = 0;
        white.insert(count);
        ++count;
    }
    
    for(int i = 0; i < graph.size(); ++i)
    {
        if(visited[i] == true)
        {
            continue;
        }
        else
        {
            visited[i] = true;
            checkCycle(i, visited,white, black, gray);
        }
    }
}

int main()
{
    std::unique_ptr<Graph> graphSmartPtr(new Graph(10));
    graphSmartPtr->addEdge(0, 1);
    graphSmartPtr->addEdge(0, 2);
    graphSmartPtr->addEdge(1, 3);
    graphSmartPtr->addEdge(1, 4);

    graphSmartPtr->addEdge(2, 5);
    graphSmartPtr->addEdge(2, 6);
    //graphSmartPtr->addEdge(3, 1);
    graphSmartPtr->detectCycleDirected();

    return 0;
}