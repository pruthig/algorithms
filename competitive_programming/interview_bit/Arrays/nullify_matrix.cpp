// Approach :
// We will use first row and first column as reference to carry out our operations.
// We will traverse the whole matrix and whenever a 0 is encountered we set A[i][0] = A[0][j] = 0
// After doing this for the first pass. we will reverse traverse the array checking the value of A[i][0] and A[0][j
#include<iostream>
#include<vector>
#include<algorithm>


using namespace std;


int main() {
	vector< vector<int> > A = {   { 0, 0},
									{ 1, 1},
								};
	
    int col0 = 1, rows = A.size(), cols = A[0].size();

    for (int i = 0; i < rows; i++) {
        if (A[i][0] == 0) col0 = 0;
        for (int j = 1; j < cols; j++)
            if (A[i][j] == 0)
                A[i][0] = A[0][j] = 0;
    }

    for (int i = rows - 1; i >= 0; i--) {
        for (int j = cols - 1; j >= 1; j--)
            if (A[i][0] == 0 || A[0][j] == 0)
                A[i][j] = 0;
        if (col0 == 0) A[i][0] = 0;
    }


	for(int i = 0; i < A.size(); ++i) {
		for(int j = 0; j < A.at(i).size(); ++j) {
			cout<<A[i][j]<< " ";
		}
		cout<<endl;
	}	
	return 0;
}
