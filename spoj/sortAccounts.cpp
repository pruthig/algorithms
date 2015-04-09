#include<string>
#include<utility>
#include<map>
#include<iostream>

using namespace std;



//Integer set for sorted ints
std::map<string, int> mp;
std::map<string, int>::iterator mpItr;

void Output(){
for(mpItr = mp.begin(); mpItr != mp.end(); ++mpItr)
{
	string tempString = mpItr->first;
	tempString.insert(2, " ");
	tempString.insert(11, " ");
	tempString.insert(16, " ");
	tempString.insert(21, " ");
	tempString.insert(26, " ");
	cout<<tempString<<" "<<mpItr->second<<endl;
}
}


int main(){

int count, entries;
int n1, n2;
int arr[6];
string s[6];
//test case
cin>>count;
//total entries
for(int j=1; j <= count; ++j)
{
	//Number of Entries for a particular test case
	cin>>entries;
	for(int m = 1; m <= entries; ++m)
	{

		string *s = new string[6];
		for(int k = 0; k < 6 ;++k)
			cin>>s[k];
		//Append all
		string main = s[0] + s[1] + s[2] + s[3] + s[4] + s[5];

		if(mp.find(main) != mp.end())
			(mp.find(main))->second++;
		else
			mp[main] = 1;
	}
	Output();
	cout<<endl;
	mp.clear();
}
return 0;
}


