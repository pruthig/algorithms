#include<iostream>
#include<vector>


using namespace std;
void print();

bool used[] = {false, false, false, false};
string s[] = { "I", "am", "a", "boy" };  
vector<string> v;
string newString = "";
int main(){
print();
return 0;
}

void print(){
if(v.size() == 4){
	for(auto a:v)
		cout<<a<<" ";
	cout<<endl;
	return;
}
for(int i=0; i<=sizeof(s); ++i){
	if(used[i])
		continue;
	else{
		//newString += s[i];
		v.push_back(s[i]);

		used[i] = true;
		print();
		//newString.erase(newString.length()-1);
		v.pop_back();
		used[i] = false;
		
}
used[i] = false;
}
}


