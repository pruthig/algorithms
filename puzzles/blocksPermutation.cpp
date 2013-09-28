//The puzzle is a solution to a problem .. where you have to fill
//2 X N matrix of blocks of size 2X2,2X1,1X2 which are following...
/*           ____
  ----  --  |____|
  |  |  ||
  ----  -- 
*/

#include<iostream>


using namespace std;


void add(int adder, int counter, const int & totalCount);
int solutions = 0;

int main()

	int totalCount;
	cout<<"Enter the count"<<endl;
	cin>>totalCount;
	add(0, 0, totalCount);
	cout<<"\nTotal Solutions to to problem are :"<<solutions<<endl;;
	return 0;
}

void add(int adder, int counter, const int & totalCount)
{
	if(counter+adder == totalCount){
		solutions++;
		return;
	}
	else if(counter+adder<totalCount){
		counter = counter + adder;
	}
	else
		return;
	
	add(1, counter, totalCount);
	add(2, counter, totalCount);
	add(2, counter, totalCount);
	return;
}
