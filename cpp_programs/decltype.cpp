//decltype helps you identify the type of entity you will supply
#include<iostream>


using namespace std;

int func(int a, int b){
return a*b;
}

int main(){
int (*p)(int, int) = &func;
int i = 33;
decltype(i) j = 44;
cout<<j<<endl;
//decltype(func) s = p(3, 4); 
int s = p(3,4);
cout<<"Value returned by function is "<<s<<endl;
return 0;
}
