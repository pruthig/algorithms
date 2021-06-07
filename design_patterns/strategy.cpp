//Strategy pattern involves choosing the appropriate algorithm at runtime

#include<iostream>
#include<string>

using namespace std;

class strategy;

class laplace : public strategy{
public:
	void call(){
		cout<<"Laplace transform called\n";
	}
};

class bezier : public strategy{
public:
	void call(){
		cout<<"Bezier curve algo called\n";
	}
};

class client{
	strategy st;
public:
	strategy getStrategy(){
		return st;
	}
};

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