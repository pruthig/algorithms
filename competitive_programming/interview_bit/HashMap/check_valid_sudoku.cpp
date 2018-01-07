// Determine if a Sudoku is valid, according to: http://sudoku.com.au/TheRules.aspx
// The Sudoku board could be partially filled, where empty cells are filled with the character ‘.’.
// The input corresponding to the above configuration :
// ["53..7....", "6..195...", ".98....6.", "8...6...3", "4..8.3..1", "7...2...6", ".6....28.", "...419..5", "....8..79"]

// To solve it, we need to check both row wise column wise and in a square as well
#include<iostream>
#include<set>
#include<vector>

using namespace std;

namespace {
    set<char> st{};
};

int validate_string(string s) {
    for(auto& a : s) {
        if(a == '.')
            continue;
        if(st.find(a) == st.end())
            st.insert(a);
        else {
            st.clear();
            return 0;  // Invalid string
        }
    }
    st.clear();
    return 1;
}

string create_string(vector<string>& vec, int x, int y) {
    string str = "";
    for(int i = x; i < x+3; ++i) {
        for(int j = y; j < y+3; ++j) {
            str += vec[i][j];
        }
    }
    return str;
}

int main() {

    vector<string> vec{"53..7....", "6..195...", ".98....6.", "8...6...3", "4..8.3..1", "7...2...6", ".6....28.", "...419..5", "....8..79"};
    // Validate row-wise (All 9)
    for(auto& a : vec) {
        if(validate_string(a) == 0)
            return 0;
    }
    // Rows validated
    // Validate column
    for(int i = 0; i < 9; ++i) {
        string temp = "";
        for(int j = 0; j < 9; ++j) {
            temp += vec[j][i];
        }
        if(validate_string(temp) == 0)
            return 0;
    }
    // Validate 9 boxes...just pass upper left coordinate .. given function will create a string which
    // will be validated thereafter
    for(int i = 0; i < 9; i+=3) {
        for(int j = 0; j < 9; j+=3) {
            string str = create_string(vec, i, j);
            if(validate_string(str) == 0)
                return 0;
        }
    }

    // Everything validated...return 1
    cout<<"A valid sudoku\n";
    return 1;
}









