//Implementation of strategy pattern...
#include<iostream>
#include<string>
#include"strategy.h"

using namespace std;

class strategy{
public:
virtual void call();

strategy* getTransform(string s){
if(s == "laplace")
	return new laplace;
else
	return new bezier;
}

};

int main(){
client c;
c.getStrategy.getTransform("bezier").call();
return 0;
}

