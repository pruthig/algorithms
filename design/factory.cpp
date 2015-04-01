//Representative factory class

#include<iostream>

using namespace std;


class shape{
public:
virtual void display(){
cout<<"This is shape";
}
static shape* getInstance(int p);
};

class circle : public shape{
void display(){
cout<<"This is circle shape"<<endl;
}
};

class square : public shape{
public:
void display(){
cout<<"Thi is square shape"<<endl;
}
};

shape* shape::getInstance(int p){
if(p == 1)
	return new circle;
else
	return new square;
}

int main(){
shape* s= shape::getInstance(1);
s->display();
return 0;
}
