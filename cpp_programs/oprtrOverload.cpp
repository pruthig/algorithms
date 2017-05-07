//Overloaded operators '+' and '>>'
#include<iostream>
#include<string>


using namespace std;


class str{

string s;
public:
str(string p):s(p){}
str(){}
string& getS();
void setS(string& s);
str operator+(str p);
friend void operator<<(ostream& o, str x);
};

str str::operator+(str p){
str temp;
temp.s = (this->s).append(p.getS());
return temp;
}

void operator<<(ostream& o, str x){
o<<x.getS()<<endl;
}

void str::setS(string& x){
this->s = x;
}	

string& str::getS(){
return this->s;
}

int main(){

str s1("gaurav");
str s2("pruthi");

str s3 = s1 + s2;
cout<<s3;
return 0;
}

