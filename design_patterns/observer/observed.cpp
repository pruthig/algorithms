#include<iostream>
#include "observed.h"

using namespace std;

void shop::changePrice(int p){
price = p;
}

void shop::notify(int c){
changePrice(c);
cout<<"Changed price after updation "<<c<<" for the entity "<<name<<endl;
}

shop::shop(){}

shop::shop( string s, int p){
name = s;
price = p;
}

