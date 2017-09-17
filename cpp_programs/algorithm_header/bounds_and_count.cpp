#include<iostream>
#include<algorithm>
#include<iterator>
#include<vector>
#include<functional>
using namespace std;


int main() {
	vector<int> v{ 1, 3, 5, 5, 5, 8, 9, 5, 23, 45, 45, 79, 98, 101};
	cout<<"original sequence\n";
	for_each(v.begin(), v.end(), [](int i) { 
		cout<<i<<" ";
	});
	int c = count(v.begin(), v.end(), 5);
	cout<<"Number of occurences for 5 are: "<<c<<endl;
	auto low = lower_bound(v.begin(), v.end(), 10);
	auto high = upper_bound(v.begin(), v.end(), 80);
	cout<<"\nLower bound for 10 is: "<<low-v.begin()<<endl;
	cout<<"\nUpper bound for 80 is: "<<high-v.begin()<<endl;
	return 0;
}
	
