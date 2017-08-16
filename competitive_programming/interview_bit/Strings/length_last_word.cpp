#include<iostream>

using namespace std;


int main() {
	string s = "one ore   ";
	int new_length = 0;
	int old_length = 0;
	for(int i = 0; i < s.size(); ++i) {
		if(s[i] != ' ')
			new_length++;
		else {
			old_length = new_length>0?new_length:old_length;
			new_length = 0;
		}
	}
	cout<<"Length of last word is: "<<(new_length>0?new_length:old_length)<<endl;
}
