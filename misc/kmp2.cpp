#include<iostream>
#include<string>
#include<cstring>


using namespace std;


int main(){
const char *s = "abacab ab";
string orig(s);
for(int i = 1; i < strlen(s) ; ++i){
	string p(s, i);
	cout<<"New string is :"<<p<<endl;
	//search for this string and check if position is 
	//Example for a b a c a b length is 6
	for(int j = 1; j < strlen(p) ; ++j){
		string subPart(p, j);
		int foundpos = orig.rfind(p, 
		if(foundpos == -1 || (found_pos+i+1) != orig.length)
			continue;
		else {
			final[i] = 
			count = i+1;
		}
	}
}
return 0;

}
	
