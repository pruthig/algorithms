//Template method pattern define a series of methods that are common to subclasses for a specific implementation
//Take the example of 2 classes crow and cook both chack and both walk and both have eyes..
//Here chack method is different for both so we write this method separately while maintaining the same name in
//the superclass
#include<iostream>

using namespace std;

class bird{
public:
void chack();
void eat(){
cout<<"We eat in a similar way\n";
}
};


class crow : public bird{
public:
void chack(){
cout<<"I chack in bitter way\n";
}
};

class cuckoo : public bird{
public:
void chack(){
cout<<"I chack in a sweet way\n";
}
};

int main(){
cuckoo ck;
ck.chack();
ck.eat();
return 0;
}

