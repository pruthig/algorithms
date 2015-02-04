//Program to print matrix in spiral order

#include<iostream>


using namespace std;

void printRightDown(int, int, int, int);
void printLeftUp(int x1, int y1, int x2, int y2);

char a[][4] = {
		
		{ 'a', 'b', 'c', 'd' },
		{ 'l', 'm', 'n', 'e' },
		{ 'k','p', 'o', 'f' },
		{ 'j', 'i', 'h', 'g' }
		
		/*
		
		{'a', 'l', 'k', 'j'},
		{'b', 'm', 'p', 'l'},
		{'c', 'n', 'o', 'h'},
		{'d', 'e', 'f', 'g'}
		*/
	     };


int main(){

printRightDown(0, 0, 3, 3);
return 0;
}


void printRightDown(int x1, int y1, int x2, int y2){

for(int i = y1; i <= y2; ++i){
	//cout<<"a["<<i<<"]["<<y1<<"]"<<a[i][y1]<<endl;;
	cout<<a[x1][i]<<", ";
}

//print down

for(int j = x1+1; j <= x2; ++j){
	cout<<a[j][y2]<<", ";
}

if(x2-x1 > 0)
	printLeftUp(x2, y2-1, x1+1, y1);

}

void printLeftUp(int x1, int y1, int x2, int y2){

for(int i  = y1; i>= y2; --i){
	cout<<a[x1][i]<<", ";
}

//print up
for(int j = x1-1; j >= x2; --j){
	cout<<a[j][y2]<<", ";
}
if(x1-x2>0)
	printRightDown(x2, y2+1, x1-1, y1);
}

