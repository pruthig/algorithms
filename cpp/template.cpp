#include<iostream>

using namespace std;


//TYPE refers to any arbitrary type.
template<class TYPE>
class Printer{
TYPE t;
public:
void print();
void setValue(TYPE arg);
};

template<class TYPE>
void Printer<TYPE>::print(){
	cout<<"Value asked is :"<<t<<endl;
}

template<class TYPE>
void Printer<TYPE>::setValue(TYPE t){
	this->t = t;
}

int main(){
Printer<char> cP;
cP.setValue('w');
cP.print();

Printer<int> iP;
iP.setValue(200);
iP.print();

return 0;
}
