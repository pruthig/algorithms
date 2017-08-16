#include <algorithm>
#include <vector>
#include <iostream>
 
 using namespace std;
 

// Is a larger
// let and b be the numbers a 
// sort using a custom function object
struct comparator{

    bool operator()(int a, int b)
    {   
    	int n_a = 0;
    	int n_b = 0;
    	string s_a = to_string(a);
    	string s_b = to_string(b);
    	
    	long converted_a = stoi(s_a + s_b);
    	long converted_b = stoi(s_b + s_a);
    	if(converted_b > converted_a)
    		return false;
    	else
    		return true;
    }   
}customLess;
    
int main() {
	vector<int> vec { 3, 30, 34, 5, 9 };
	string s = "";
	std::sort(vec.begin(), vec.end(), customLess);
	for(auto a : vec) {
		s.append(to_string(a));
	}
	cout<<s<<" ";
}
