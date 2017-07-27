#include<iostream>
#include<string>
#include<algorithm>

using namespace std;


int main() {
	string d = "A man, a plan, a canal: Panama";
	bool flag = true;
	d.erase( std::remove_if( d.begin(), d.end(), []( char c ) { return !std::isalnum(c) ; } ), d.end() ) ;
	int i = 0; int j = d.size()-1;
	while(i <= j) {
		if(tolower(d[i]) != tolower(d[j])) {
			flag  = false;
			break;
		}
		else {
			i++; --j;
		}
	}
	if(flag)
		cout<<"String is palindrome";
	else
		cout<<"String is not palindrome";
}
