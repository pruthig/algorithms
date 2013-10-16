/*The game shows a number in a square which tells you how many mines there are
adjacent to that square. Each square has at most eight adjacent squares. The 44 field
on the left contains two mines, each represented by a * character. If we represent the
same field by the hint numbers described above, we end up with the field on the right:
*/


#include<iostream>
#include<cstdlib>


using namespace std;

//Global array for accessibility across all functions
int m, n;


char checkCounter( char **a, int x, int y)
{
int counter = 0;                                                 //Counter for input index
if(a[x][y] == '*')
return '*';

for(int i=x-1;i<=x+1;i++)
{
	for(int j=y-1;j<=y+1;j++)
	{
		if((i==x && j==y) || i<0 || j<0 || i>m-1 || j>n-1 )
		continue;
		if(a[i][j] == '*')
			counter++;
	
	}
}
if(x == 1 && y == 0)
cout<<counter<<endl;
return (char)(((int)'0')+counter);
}

int main()
{
cout<<"Enter the dimension of game\n";
cin>>m>>n;
char **a = (char**)malloc(m*sizeof(char*));
for(int i=0;i<m;i++)
a[i] = (char*)malloc(n*sizeof(char));

cout<<"\nEnter the matrix in \"*\" and \"'\" form "<<endl;
for(int i=0;i<m;i++)
	for(int j=0;j<n;j++)
		cin>>a[i][j];

for(int i=0;i<m;i++)
	for(int j=0;j<n;j++)
		a[i][j] = checkCounter(a, i, j);

//Now print the same
for(int i=0;i<m;i++)
{
	for(int j=0;j<n;j++){
		cout<<" "<<a[i][j]<<",";
	}
	cout<<endl;
}


return 0;
}
