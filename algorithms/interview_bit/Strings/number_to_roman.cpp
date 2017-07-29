#include<iostream>
#include<map>
#include<string>
#include<cmath>

using namespace std;

int main() {
	int number = A;
	map<int, string> mp;
	mp[1] = "I";
	mp[5] = "V";
	mp[10] = "X";
	mp[50] = "L";
	mp[100] = "C";
	mp[500] = "D";
	mp[1000] = "M";
	
	string s = std::to_string(number);
	int max_power = s.size()-1;
	string total = "";
	for(int i = pow(10, max_power); number && i >= 1;  i /= 10)
	{
		string partial = "";
		int res = number/i;
		int temp = res;

		if(temp == 9) {
			partial = partial + mp[i] + mp[10*i];
		}
		else if (temp >= 5) {
			partial = partial + mp[5*i];
			temp = temp-5;
		}
		else {
		}
		
		if(temp == 4) {
			partial = partial + mp[i] + mp[5*i];
		}

		if(temp < 4){
			while(temp != 0) {
				partial += mp[i];
				--temp;				
			}
		}
		number = number - res*i;
		total += partial;
	}
	cout<<"String is : "<<total<<endl;
}
