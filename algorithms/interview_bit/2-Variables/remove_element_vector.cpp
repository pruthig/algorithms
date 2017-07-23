#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

int main(){
	
	vector<int> A{4, 5, 6, 1, 2, 1, 3, 1, 90};
	int B = 1;
	A.erase(std::remove(A.begin(), A.end(), B), A.end());
	cout<<"Size is: "<<A.size()<<endl;
	return 0;

}
