#include<iostream>
#include<utility>
#include<list>
#include<string>
#include<map>
#include<list>

using namespace std;


int generateValue(string s);


map<int, list<string> > m;

std::pair < int, list<string> > mP;
list<string> gList;
list<string>::iterator lItr;
map<int, list<string> >::iterator mItr ;


string str[] = { "act", "pal", "lau", "maj", "mat", "pol", "owl", "low", "plo", "lap" };
//create map of integer and lists

int prime[26] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101 };

int main(){

int gN;
//insertion in map by searching..if exists then append to the list else insert new by making pair
for(int i = 0; i<10; ++i){
	gN = generateValue(str[i]);
        if( ( mItr = m.find(gN) ) == m.end() ){
		list<string> l;
		l.push_back(str[i]);
		m.insert( std::make_pair< int, list<string> >(gN, l) );
	}
	else{
		mItr = m.find(gN);
		mItr->second.push_back(str[i]);
	}
}

for(mItr = m.begin(); mItr != m.end(); ++mItr){
	gList = mItr->second;
	for(lItr = gList.begin(); lItr != gList.end(); ++lItr)
		cout<<(*lItr)<<", ";
	cout<<endl;
}
return 0;
}
	


int generateValue(string s){
int sz = s.size();
int result = 1;

for(int i = 0; i<sz; ++i){
	result = result * prime[ s[i]-'a'];
}
cout<<"Returning result "<<result<<endl;
return result;
}
	




