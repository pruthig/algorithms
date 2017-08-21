#include<iostream>
#include<algorithm>
#include<string>
#include<vector>

using namespace std;

int main() {
	string a = "111";
	string b = "111";
	string res = "";
	int sum = 0;
	
	
	int carry = 0;
	reverse(a.begin(), a.end());
	reverse(b.begin(), b.end());
	int i = 0, j = 0;
	while(1) {
		sum = 0;
		if(i >= a.size() && j >= b.size())
			break;
		if(i < a.size())
			sum = sum + (a[i] - '0'); 
		if(j < b.size())
			sum = sum + (b[i] - '0'); 
		sum += carry;
		
		if(sum == 2) {
			sum = 0; 	carry = 1;
		}
		else if(sum == 3) {
			sum  = 1; carry = 1;
		}
		else {
			carry = 0;
		}
		
		res = res + to_string(sum);
		++i; ++j;
	}
	if(carry == 1)
		res = res + "1";
	reverse(res.begin(), res.end());
	cout<<"Result is: "<<res<<endl;
}
