//The puzzle is a solution to a problem .. where you have to fill
//2 X N matrix of blocks of size 2X2,2X1,1X2 which are following...
/*           ____
  ----  --  |____|
  |  |  ||
  ----  -- 
*/

#include<iostream>
#include<fstream>


using namespace std;


void add(long adder, long counter, const long & totalCount, long mod_param);
int solutions = 0;

ifstream inputFile("sample.txt");

int main()
{
	long testCases, totalCount, mod_param;
	inputFile>>testCases;
	cout<<"Number of test cases: "<<testCases<<endl;
	for(int i=0; i<testCases;i++)
	{
		inputFile>>totalCount;
		cout<<"Count entered :"<<totalCount;
		inputFile>>mod_param;
		cout<<"MOD_PARAM read :"<<mod_param<<endl;
       		add(0, 0, totalCount, mod_param);
		cout<<solutions<<endl;
		solutions =0;
	}
	return 0;
}

void add(long adder, long counter, const long & totalCount, long mod_param)
{
	if(counter+adder == totalCount){
		solutions++;
		solutions = solutions%mod_param;
		return;
	}
	else if(counter+adder<totalCount){
		counter = counter + adder;
	}
	else
		return;

	add(1, counter, totalCount, mod_param);
	add(2, counter, totalCount, mod_param);
	add(2, counter, totalCount, mod_param);
	return;
}
