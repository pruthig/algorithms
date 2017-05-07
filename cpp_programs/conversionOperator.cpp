#include<iostream>

using namespace std;

class Test
{
	int x;
public:
	//user defined ctor...
	Test():x(32){}
	//Conversion operator
	operator double(){
		return x;
	}
};

//Conversion operator is used when the class object needs to be assigned to a primitive type...
//It is reverse of the Single argument constructor that creates new object but it downgrades..

//While dealing with C calls these are unavoidable... like for this
//strcpy(dest, orig)
//where orig is class type *pointer
int main(){	
	Test t;
	double p = t;
	cout<<"Value of p is "<<p<<endl;
	return 0;
}
