//Flyweight pattern is basically used to achieve memory efficiency when a large number
//of objects are created and shared in a program...time efficieny might take a hit in 
//flyweight pattern...

#include<iostream>
#include<cstdlib>
#include<ctime>
#include<chrono>
#include<map>

using namespace std;
using namespace std::chrono;

class Circle{

	int radius;
public:
 	Circle(){}
	Circle(int n):radius(n) {}
};

std::map<int, Circle*> mCircle;


int main(){
std::srand(std::time(0));  // needed once per program run
auto start=system_clock::now();
/*
for(int i = 0; i < 1000; ++i) {
	int r = std::rand() % 10 + 1;
    Circle *c = new Circle(r);
}
*/
for(int i = 0; i < 1000; ++i) {
	int r = std::rand() % 10 + 1;
	Circle *c ;
	auto searcher = mCircle.find(r) ;
    if(searcher != mCircle.end())
		c = searcher->second;
	else {
    	c = new Circle(r);
		mCircle.insert(std::make_pair(r, c));
	}
}



auto end=system_clock::now();
cout<<"Time elapsed : "<<(end - start).count()<< endl;
return 0;
}
