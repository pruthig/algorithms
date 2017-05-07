#include<iostream>
using namespace std;

class Base{
public:
	virtual void show(){
		cout<<"In base";
	}
};

class Derived : public Base{
public:
	void show(){
		cout<<"In derived\n";
	}
};

int main(){

	Base *bp = new Derived;
	bp->show();
	return 0;
}
