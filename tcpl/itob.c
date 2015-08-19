//integer n -> b representation in a  string
#include <stdio.h>
#include<string.h>


int main(){
int n;
char s[32];
printf("Please enter the number : ");
scanf("%d", &n);

int i = 0;
while( n%16 != 0 ){
	if( n%16 >= 0 && n%16 <= 9)
		s[i++] = '0' +  n%16;
	else
		s[i++] =  'a' + n%16 - 10;
	n/=16;
}
s[i] = '\0';

//in place reverse
int start = 0;
int end = strlen(s)-1;
char c;
while(start < end){
char c;
c = s[start];
s[start] = s[end];
s[end] = c;

++start;
--end;
}

printf("%s\n", s);
return 0;
}	
