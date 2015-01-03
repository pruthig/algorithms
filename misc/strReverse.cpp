#include<iostream>
#include<string.h>

int main(){


char p[] = "amba";
int i = 0;

int j = strlen(p) - 1;
char t;


while ( i < j ){
	t = p[i];
	p[i] = p[j];
	p[j] = t;
	++i;--j;
}
std::cout<<"Reversed is "<<p<<std::endl;
return 0;
}
	
