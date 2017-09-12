// Subsets
#include<iostream>
#include<vector>
#include<string>

using namespace std;

vector< vector<int> > vecS{};

void generator(int index,vector<int> &A,vector<int> tempAns)
{
    for(int i=index;i<A.size();i++)
    {
        tempAns.push_back(A[i]);
        vecS.push_back(tempAns);
        generator(i+1,A,tempAns);
        while(i<A.size()-1 && A[i] == A[i+1])
            i++;
        tempAns.pop_back();
    }
}
int main() {
	vector<int> vecI{ 1, 2, 3, 4};
	vector<int> t{};
	generator(0, vecI, t);
	for(auto a : vecS) {
		for(auto b : a)
			cout<<b<<" ";
		cout<<endl;
	}

	return 0;

}

