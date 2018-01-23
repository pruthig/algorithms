// Sudoku Solver
// Brute-force algorithm which uses permutation to find the possible answer
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>

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


int isValidSudoku(vector<string> &A) {
    vector<string> vec{A};
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
    //cout<<"A valid sudoku\n";
    return 1;
    
}

void fillPermutation(string partial, string& full) {
	int i = 0, j = 0;
	string result = "";
	while(i < full.size() && j < partial.size()) {
		if(full[i] == '.') {
			full[i] = partial[j];
			++j;
		}
		++i;
	}
}

// A string containing '.' is passed and first permutation is sought
string getFirstPermutation(string str) {
	set<char> s{};
	string result = "";
	for(auto a : str) {
		if(a != '.') 
			s.insert(a);
	}
	for(char i = '1' ;i <= '9'; ++i) {
		if(s.find(i) == s.end())
			result += i;
	}
	sort(result.begin(), result.end());
	return result;
}

void printSudoku(vector<string>& vec) {
	for(auto a : vec) {
		for(auto b : a)
			cout<<b<<" ";
		cout<<endl;
	}
}

bool createValidSudoku(int index, vector<string> input) {
	if(index == 9) {
		if(isValidSudoku(input)) {
			printSudoku(input);
			return true;
		}
		else
			return false;
	}
	// Get First permutation
	string str = getFirstPermutation(input[index]);
	
	do {
		// Partial contains the permutation generated and full refers to the string containing '.'
		string backed = input[index];
		fillPermutation(str, input[index]);
		cout<<"string and index are: "<<input[index]<<" "<<index<<endl;
		if(createValidSudoku(index+1, input))
			break;
		input[index] = backed;
	}while (next_permutation(str.begin(), str.end()));
	return false;
	
}
int main() {
	vector<string> A{ {"53..7...."}, {"6..195..."}, {".98....6."}, {"8...6...3"}, {"4..8.3..1"}, {"7...2...6"}, {".6....28."}, {"...419..5"}, {"....8..79"} };
	if(createValidSudoku(0, A))
		cout<<"Valid solution found";
	return 0;
}

