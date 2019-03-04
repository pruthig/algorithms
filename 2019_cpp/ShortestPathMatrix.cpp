// Shortest path in a matrix from top left to bottom right
#include<iostream>
#include<queue>
#include<limits.h>
#include<cstdlib>
#include<cstring>


using namespace std;

struct Point
{
    int x;
    int y;
};

namespace {

    int xmod[] = {-1, 0, 0, 1};
    int ymod[] = { 0, -1, 1, 0};

    const int ROW = 5;
    const int COL = 5;
    int grid[ROW][COL] = 
    { 
        31, 100, 65, 12, 18, 
        10, 13, 47, 157, 6, 
        100, 113, 174, 11, 33, 
        88, 124, 41, 20, 140, 
        99, 32, 111, 41, 20 
    }; 
    bool visited[ROW][COL]; 
    unsigned int weight[ROW][COL];
}

bool isValid(int x, int y)
{
    if((x >= 0 && x < ROW) && (y>=0 && y<COL))
        return true;
    else
        return false;
}


int findShortestCost()
{
    queue<Point> q{};
    Point p;

    p.x = 0; p.y = 0;
    q.push(p);

    memset(weight, INT_MAX, sizeof(weight[0][0]) * ROW * COL);
    memset(visited, false, sizeof(visited[0][0]) * ROW * COL);

    // set weight of first to 0
    weight[0][0] = grid[0][0];

    
    while(!q.empty())
    {
        Point temp_pt = q.front();
        q.pop();

        for(int i = 0; i < 4; ++i)
        {
            int x_coord = temp_pt.x + xmod[i];
            int y_coord = temp_pt.y + ymod[i];
            if(isValid(x_coord, y_coord))
            {
                if(weight[temp_pt.x][temp_pt.y] + grid[x_coord][y_coord] < weight[x_coord][y_coord])
                {
                    weight[x_coord][y_coord] = weight[temp_pt.x][temp_pt.y] + grid[x_coord][y_coord];
                    Point t_pt { x_coord, y_coord };
                    // Push the node again if its weight has reduced because it might reduce other'w weight too.
                    q.push(t_pt);

                }
            }
        }
        visited[temp_pt.x][temp_pt.y] = true;
    }

    /*
    for(int i = 0; i < ROW; ++i)
    {
        for(int j = 0; j < COL; ++j)
            cout<<weight[i][j]<<" ";
        cout<<endl;
    }
    */
    return weight[ROW-1][COL-1];
    
}

int main()
{
    cout<<"Shortest path cost is: "<<findShortestCost()<<endl;
    return 0;
}
