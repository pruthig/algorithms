#include<iostream>
#include<string>
#include<forward_list>
#include<map>

using namespace std;
//find occurences of string and corresponding count
void checkOccurence(string *sArr, int size){
	map<string, forward_list<int> > mMap;
	map<string, forward_list<int> >::iterator mItr;
	forward_list<int> fList;
	forward_list<int>::iterator fItr;

	for(int i = 0; i < size; ++i){
			mItr = mMap.find(sArr[i]);
			//If string doesn't exist et al
			if(mItr == mMap.end())
			{  //insert
				forward_list<int> dList;
				dList.push_front(i+1);
				mMap.insert( std::make_pair(sArr[i], dList));
				continue;
			}
			//it exists , simply add index to the list corr. to the string given ...
			else
			{
				
				(*mItr).second.push_front(i+1);
				
			}
	}
				
			//printer
	for(mItr = mMap.begin(); mItr != mMap.end(); ++mItr){
		cout<<"String :   "<<(*mItr).first<<'\t';
		forward_list<int> dList ( (*mItr).second );
		for(fItr = dList.begin(); fItr != dList.end(); ++fItr){
			cout<<*fItr<<", ";
		}
			cout<<endl;
	}
}


int main(){

string s[] = { "gau", "prut", "wild", "prut"};
checkOccurence(s, 4);
return 0;
}




