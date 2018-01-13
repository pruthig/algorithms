/* This algorithm is most famous N-queen puzzle */
#include<iostream>
#include<vector>

#define DIM 4

using namespace std;

namespace {
    vector<vector<string>> visual{};
};

int isDiagSafe(int i, int j, vector<int>& row, vector<int>& col, vector<vector<int>>& visited  ) {

    int hor = 1;
    for(int x = i-1, y = j-1; x >= 0 && y >= 0; --x, --y) {
        if(visited[x][y] == 1)
            return 0;
    }

    for(int x = i-1, y = j+1; x >= 0 && y >= 0 && x < DIM && y < DIM; --x, ++y) {
        if(visited[x][y] == 1) 
            return 0;
    }
    return 1;
}
// This function gets 1 solution and adds to vector<string>
void createSolution(vector<pair<int, int>>& res) {
    vector<string> vec{};
    // Number of entries will be equal to number of dimensions
    for(int i = 0; i < DIM;  ++i) {
        string str(DIM, '.');
        vec.push_back(str);
    }
    // Now fill
    for(auto a : res) {
        vec[a.first][a.second] = 'Q';
    }
    visual.push_back(vec);
    
}

void printResult(vector<pair<int, int>>& res) {
    static int count = 0;
    for(auto a : res) {
        cout<<a.first<<" "<<a.second<<endl;
    }
    ++count;
    cout<<"count is: " <<count<<endl;
}

void n_queen_solution(vector<int>& row, vector<int>& col, vector<pair<int, int>>& res, vector<vector<int>> & visited, int i) {
    if(i == DIM) {
        //printResult(res);
        createSolution(res);
        return;
    }

    for(int j = 0; j < DIM; ++j) {
        if(row[i]  == 1 || col[j] == 1   || !isDiagSafe(i, j, row, col, visited))
            continue;
        else
            row[i] = 1; col[j] = 1;

        visited[i][j] = 1;
        res.push_back(std::make_pair(i, j));
        //cout<<"Calling new function with i and j : "<<i<<"  "<<j<<endl;
        n_queen_solution(row, col, res, visited, i+1);
        //cout<<"returng with i : "<<i<<" and j: "<<j<<endl;
        row[i] = 0; col[j] = 0;
        visited[i][j] = 0;
        res.pop_back();
    }

}

// Main method
int main() {
    visual.clear();
    vector< vector<int> > visited{};
    for(int i = 0; i < DIM; ++i) {
        vector<int> tmp(DIM, 0);
        visited.push_back(tmp);
    }

    vector<int> col(DIM, 0); // = {};
    vector<int> row(DIM, 0);
    // vector of pair to store the result
    vector< pair<int, int> > res{};
    n_queen_solution(row, col, res, visited, 0);

    cout<<"Size of visual is: "<<visual.size()<<endl;
    for(auto a : visual) {
        for(auto b : a) {
            cout<<b<<endl;
        }
        cout<<endl;
    }
    return 0;
}

