#include<iostream>
#include<algorithm>
#include<vector>
#include<string>
#include<initializer_list>

using namespace std;

typedef struct emp{
string name;
int id;
}emp;

std::vector<emp> vec{ {"gaurav", 34}, {"amri", 22}, {"samna", 1}, {"pap", 89}, {"barb", 11} };

class sortStruct{
public:
bool operator()(struct emp &e1, struct emp &e2){
	if( e1.name.compare(e2.name) < 0)
		return true;
	else
		return false;
}
};

void tryMe(){

for(auto t:vec){
	cout<<"Value is "<<t.name<<endl;
}
}

int main()
{
	sort(vec.begin(), vec.end(), sortStruct());
	tryMe();
    return 0;
}
