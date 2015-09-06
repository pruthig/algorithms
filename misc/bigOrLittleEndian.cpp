//Program to find whether machine is little endian or big endian
#include<iostream>

using std::cout;

int main(){
int i = 1;
char *c = (char*)&i;

if(*c)
	cout<<"Little Endian\n";
else
	cout<<"Big Endian\n";

return 0;
}
