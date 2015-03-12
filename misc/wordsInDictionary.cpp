#include<iostream>
#include<fstream>

using namespace std;


ifstream inputFile("../puzzles/american-english");
string strArray[99155];
string line = "";

int main(){

int index=0;
char inputArray[16];
while(inputFile.good() && index<99155){
	getline(inputFile, line);
	for(int i=0;i<line.length();i++)
		if(line.at(i) == ' ')
	        {
		    cout<<"Dictionary invalid, please select some other one"<<endl	;
	            return -1;
		}
        strArray[index] = line;
	index++;

}

cout<<"Loaded words in memory\n";
cout<<"Please enter the input string"<<endl;
return 0;
}


void searchDict(string s, int start, int end){

    if(start>end)
        return;

    int mid = (start+end)/2;
    if(s.compare(strArray[mid]) > 0)
    {
        start=mid+1;
        searchDict(s, start, end);
    }
    else if(s.compare(strArray[mid]) < 0)
    {
        end=mid-1;
        searchDict(s, start, end);
    }
    else
    {
	cout<<"Word found";
    }
	
}
