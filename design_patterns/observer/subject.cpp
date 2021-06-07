//Main class that will hold the list
//Subject class basically has methods to register and unregister the observers and a method to notify
//those observers
#include<iostream>
#include "observed.h"
#include<string>
#include<vector>
using namespace std;



class subject{

vector<shop*> list;
public:
void attach(shop*);
void detach(shop*);
void notify(int price);
void changePriceOfSubject(int n);
};


void subject::attach(shop* s){
	list.push_back(s);
}

void subject::detach(shop *s){
for(vector<shop*>::iterator itr = list.begin(); itr != list.end(); ++itr)
	if(*itr == s){
		list.erase(itr);
		cout<<"Shop Erased";
	}
}

void subject::notify(int price){
for(auto a:list)
	a->notify(price);
}

void subject::changePriceOfSubject(int n){
	notify(n);
}



int main(){
//Main sibject class
subject s;
shop *s1 = new shop("Gaurav", 32);
shop *s2 = new shop("Tarun", 79);
s.attach(s1);
s.attach(s2);
s.changePriceOfSubject(80);   //changePrice will impact and modify prices of all commodities in another case we can have 
//certain set of variables that are mutable and you generate the notification call when any of the mentioned 
//variables get changed...
return 0;
}

