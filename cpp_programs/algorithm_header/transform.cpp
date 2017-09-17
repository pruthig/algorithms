#include<iostream>
#include<algorithm>
#include<iterator>
#include<vector>
#include<functional>
using namespace std;


int main() {
	vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8};
	vector<int> output{};
	output.resize(v.size());
	cout<<"original sequence\n";
	for_each(v.begin(), v.end(), [](int i) { 
		cout<<i<<" ";
	});
	transform(v.begin(), v.end(), output.begin(), [](int i) { 
		return i+2;
	});
	cout<<"\no/p after unary opn.(add 2 to each element) is applied to 1 sequence\n";
	for_each(output.begin(), output.end(), [](int i) { 
		cout<<i<<" ";
	});
	// Now 'output' will contain same number of elements,let's apply a binary operation
	vector<int> output2(8);
	transform(v.begin(), v.end(), output.begin(), output2.begin(), plus<int>());
	cout<<"\no/p after binary opn.(add elements of sequences) is applied to 2 sequences\n";
    //std::copy(output2.begin(), output2.end(),ostream_iterator<char>(std::cout, " "));
    for_each(output2.begin(), output2.end(), [](int i) { 
		cout<<i<<" ";
	});
	
	return 0;
}
