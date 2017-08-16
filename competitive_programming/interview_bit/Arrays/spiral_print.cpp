/*
Input: A = 4.
Output:

4 4 4 4 4 4 4 
4 3 3 3 3 3 4 
4 3 2 2 2 3 4 
4 3 2 1 2 3 4 
4 3 2 2 2 3 4 
4 3 3 3 3 3 4 
4 4 4 4 4 4 4 
*/

#include<iostream>
#include<vector>

using namespace std;

int main() {
	int A = 1;
	int t = 0, b = A*2-2, l = 0, r = A*2-2;
	
	int **arr = (int**)malloc(sizeof(int*)*(A*2-1));
	for(int i = 0; i < A*2-1; ++i) {
		arr[i] = (int*)malloc(sizeof(int)*(A*2-1));
	}
	int direction = 0;   //0->left to right, 1->top to bottom, 2->right to left, 3->bottom to top
	int val = A;
	while(l <= r && t <= b && val != 0) {
	    // print l -> r
	    if(direction == 0) {
	        for(int i = l; i  <= r; ++i) {
	            arr[t][i] = val;
	        }
	        t++;
	        
	    }
	    // print t->b
	    else if(direction == 1) {
	        for(int i = t; i  <= b; ++i) {
	            arr[i][r] = val;
	        }
	        r--;
	        
	    }
	    // print r->l
	    else if(direction == 2) {
	        for(int i = r; i  >= l; --i) {
	            arr[b][i] = val;
	        }
	        b--;
	        
	    }
	    // print b->t
	    else {
	        for(int i = b; i  >= t; --i) {
	            arr[i][l] = val;
	        }
	        l++;
	        --val;
	        
	    }
	    
	    direction = (direction+1)%4;
	}
	std::vector<std::vector <int> > result;
	for(int i=0;i<A*2-1;i++)
	{
	    result.push_back(std::vector<int>());
	    for(int j=0;j<A*2-1;j++)
	    {    
	        result[i].push_back(arr[i][j]);    
	    }
	}

	for(auto a : result) {
		for(auto b : a)
			cout<<b<<", ";
		cout<<endl;
	}
	
	return 0;
}

