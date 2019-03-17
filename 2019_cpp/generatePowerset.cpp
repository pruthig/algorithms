#include<iostream>
#include<cmath>
#include<string>

using namespace std;


void generatePowerset(string str)
{
	int set_size = str.size();
	int power_set_size = pow(2, str.size());
	cout<<"Power set size is: "<<power_set_size<<endl;	
	for(int i = 0; i < power_set_size; ++i) {
		for(int char_index = 0; char_index < set_size; ++char_index)
			if((1<<char_index)&i)
				cout<<str[char_index];
		cout<<endl;
	}
}

int main()
{
  string str = "";
  cout<<"Enter input string\n";
  cin>>str;
  generatePowerset(str);
  return 0;
}
	
