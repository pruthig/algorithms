#include<iostream>


using namespace std;


class check{

int a ;
int b ;

public:
check(){}
check(int m, int n):a(m), b(n){}
int geta(){ return a; }
int getb(){ return b; }
friend ostream& operator<<( ostream& out, check& c){
	out<<c.geta()<<" , "<<c.getb()<<endl;
}


};



int main(){
	check c(1, 2);
	//cout.operator(
	cout<<c;
	return 0;
}

