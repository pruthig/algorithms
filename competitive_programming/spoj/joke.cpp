#include<string>
#include<iostream>

using namespace std;



//Integer set for sorted ints

int main(){

int count, entries;
int n1, n2;
//test case
cin>>count;
//total testCases
for(int j=1; j <= count; ++j)
{
	//Number of Entries for a particular test case
	cin>>entries;
	for(int m = 1; m <= entries; ++m)
	{
        cin>>n1>>n2;

	}
    int ans = 0;
    for(int i = 1; i<=entries; ++i)
        ans ^= i;
    cout<<ans<<endl;
}
return 0;
}


