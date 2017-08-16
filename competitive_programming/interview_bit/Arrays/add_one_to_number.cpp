#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;


int main() {
	vector<int> A{ 0, 0, 0, 0, 9, 9, 9 };
	int carry = 0;
	reverse(A.begin(), A.end());
	if(A.at(0) >= 0 && A.at(0) < 9) 
	{
		A.at(0)++;
	}
	else 
	{
		for(int i  = 0; i < A.size(); ++i) {
			// Adding to 9
			if(A.at(i)+1 < 10) {
				A.at(i)++;
				carry = 0;
				break;
			}
			else {
				A.at(i) = (A.at(i)+1)%10;
				carry = 1;
			}
			
		}
	}
	if(carry)
		A.push_back(1);
	
	while ( !A.empty() && A.back() == 0 ) { A.pop_back(); }
	reverse(A.begin(), A.end());
	cout<<"Number is: \n";
		for(auto a: A)
			cout<<a;
	return 0;
}
