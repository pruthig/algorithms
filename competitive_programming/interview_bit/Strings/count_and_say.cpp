#include<iostream>

using namespace std;

// Feed and it will give
string generator(string  input) {
	string output = "";
	int i = 0; 
	while(i < input.length()) {
		char c = input[i];
		int count = 1;
		while(i < input.length() && input[i] == input[i+1]) {
			++i; ++count;
		}
		output += to_string(count);
		output += input[i];
		++i;
	}
	return output;
}

int main() {
	//long long number = 1113213211;
	string number = "1";
	int n = 9;
	for(int i = 1; i < n; ++i) {
		number = generator(number);
	}
	cout<<"Final number is: "<<number<<endl;
	return 0;
}
